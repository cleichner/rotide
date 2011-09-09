//
// Copyright 2011 Justin Bruce Van Horne <justinvh@gmail.com>
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "rotide/curses.hpp"

#include <csignal>
#include <cassert>
#include <cstdlib>

// Localized danger. No big problem.
using namespace curses_lib;

// This is a bit preemptive, but we can handle "multiple" instance of
// curses so we can possible to many buffers with their own interactions,
// e.g. various windows, overlaps, etc.
namespace {

typedef std::vector<Curses*> Curses_instances;
Curses_instances instances;

} // namespace


// Whenever a SIGWINCH occurs, the instances need to know that they have
// been resized so they can respectively handle overflow of characters,
// divisions, and so on.
//
// This can potentially happen at anytime and is not thread safe.
void terminal_resized(int signal)
{
    int row, col;
    getmaxyx(stdscr, row, col);
    resizeterm(row, col);

    for (Curses_instances::iterator it = instances.begin(),
            end = instances.end();
            it != end;
            ++it)
    {
        (*it)->resize();
    }
}

Curses::Curses()
{
    int row, col;
    std::signal(SIGWINCH, terminal_resized);

    // This is an interesting observation. ESCDELAY is a global variable
    // that defines how long the program waits for a second character
    // in the ESC-<x> sequence where x is any character. Under vim,
    // this value is 25ms. So, we do the same as a default, but allow
    // the user to specify an escape delay if so they wanted to.
    if (getenv ("ESCDELAY") == NULL)
        ESCDELAY = 25;

    // Initialize ncurses
    initscr();
    start_color();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    // Create a screen buffer
    getmaxyx(stdscr, row, col);
    active_window = newwin(row - 2, col, 0, 0);
    status_window = newwin(2, col, row - 2, 0);
    touched_window = active_window;
    touchwin(active_window);
    touchwin(status_window);
    buffers.push_back(active_window);
    buffers.push_back(status_window);

    // Add to the instances for signals
    instances.push_back(this);
} 

Curses::~Curses()
{
    shutdown();
}

// The status_window bar is located at the bottom of the screen
// TODO(justinvh): For now it is a fixed location, but ideally it should
// be located whatever column specified, preferably in JavaScript.
void Curses::draw_status_bar()
{
    int row, col;
    getmaxyx(status_window, row, col);
    wmove(status_window, 0, 0);
    whline(status_window, ACS_HLINE, col);
}

// A clear will clear all windows, not just the active_window buffer.
// If a buffer wants to clear itself, then it should be done in JavaScript
void Curses::clear()
{
    for (Buffer_list::const_iterator cit = buffers.begin(),
            end = buffers.end();
            cit != end;
            ++cit)
    {
        werase(*cit);
    }
}

// Resize each of the buffers in a curses instance.
// TODO(justinvh): This really doesn't make sense.
void Curses::resize()
{
    for (Buffer_list::const_iterator cit = buffers.begin(),
            end = buffers.end();
            cit != end;
            ++cit)
    {
    }
}

// Destroys the window
// TODO(justinvh): This doesn't really make sense in the idea of
//                 multiple buffers. This means if one instance shuts
//                 down, all the instances shut down. DANGER WILL ROBINSON.
void Curses::shutdown()
{
    endwin();
}

// Refresh the active_window window. Make the curses go to the appropriate position.
void Curses::refresh()
{
    wrefresh(touched_window);
}

// Draws a line in the terminal at the current position.
// The default character is the ACS_HLINE character which is fine.
void Curses::line()
{
    whline(active_window, ACS_HLINE, pos.row);
}

// Wait will wait until the next character is pressed.
void Curses::wait()
{
    wgetch(active_window);
}

// Gets the next pressed character and returns true or false depending
// on if CTRL_C is pressed or not.
// TODO(justinvh): The CTRL_C break is a temporary thing.
bool Curses::get(char* s)
{
    *s = wgetch(active_window);
    last_key = (int)*s;
    if (*s == CTRL_C)
        return false;
    return true;
}

// Returns a curses position object at a given x, y so that input can
// be fed to the screen in an ostream-style manner.
//
// EXAMPLE: 
//  Curses_pos& pos = curses.at(0, 0);
//  pos << "Hello, world" << BOLD << "!!!! And goodbye in bold.";
//
Curses_pos& Curses::at(const int x, const int y)
{
    assert(active_window != NULL && "Active window is null.");
    pos.instance = this;
    pos.col = y;
    pos.row = x;
    pos.active = active_window;
    pos.col = pos.col == 0 ? 1 : pos.col;
    pos.row = pos.row == 0 ? 1 : pos.row;
    wmove(active_window, pos.row, pos.col);
    return pos;
}


// Returns a curses position object relative to the status_window line.
// Useful for just writing things to the user at a whim.
Curses_pos& Curses::status()
{
    int row, col;
    getmaxyx(status_window, row, col);
    spos.instance = this;
    spos.col = 0;
    spos.row = row - 1;
    spos.active = status_window;
    return spos;
}

// Prints a given string to the console at an x, y coordinate.
// This method should typically not be called directly since the
// << operators exist.
void Curses_pos::print(const std::string& s)
{
    wmove(active, row, col);
    mvwprintw(active, row, col, "%s", s.c_str());
    col += s.size();
    if (focus) instance->touched_window = active;
}

// Handles transforming a position as the character stream moves.
//
// EXAMPLE: 
//  Curses_pos& status_window = curses.status();
//  Curses_pos& warning = curses.at(5, 5);
//  warning << BOLD << COLOR(RED, BLACK) 
//          << "Fire!!" << status_window << "-- FIRE --";
//
Curses_pos& Curses_pos::operator<<(const Curses_pos& cp)
{
    col = cp.col;
    row = cp.row;
    active = cp.active;
    return *this;
}

// Handles actions into the character stream. These actions are defined
// in the Curses_action enumeration.
//
// EXAMPLE:
// Curses_pos& status_window = curses.status();
// status_window << COLOR(RED, BLACK) 
//        << "Hello, world" << RESET
//        << "Regular colors!";
//
Curses_pos& Curses_pos::operator<<(const Curses_action& ca)
{
    long int action = (long int)ca;

    // NEXT_LINE
    if (action & NEXT_LINE) {
        col = 1;
        row++;
    } 

    // RESET
    if (action & RESET) {
        for (Color_list::const_iterator 
                cit = color_ids.begin(),
                end = color_ids.end();
                cit != end;
                ++cit)
        {
            wattroff(active, *cit);
        }
        wattrset(active, NORMAL);
    } 

    // HLINE
    if (action & HLINE) {
        int mcol, mrow;
        getmaxyx(stdscr, mrow, mcol);
        mvwhline(active, row, 1, ACS_HLINE, mcol - 2);
    } 

    // CLEAR
    if (action & CLEAR) {
        wmove(active, row, 0);
        wclrtoeol(active);
    }

    // FOCUS
    if (action & FOCUS)
        focus = true;

    if (action & NOFOCUS)
        focus = false;

    return *this;
}

// Handles various predefined Curses styles, such as BOLD, to the
// character stream.
//
// EXAMPLE:
//  Curses_pos& warning = curses.at(12, 12);
//  warning << BOLD << "WARNING!" << NEXT_LINE;
//
Curses_pos& Curses_pos::operator<<(const Curses_style& cs)
{
    wattron(active, cs);
    return *this;
}

// Handles color objects being put into the character stream.
//
// EXAMPLE:
// Curses_pos& warning = curse.at(12, 12);
// Curses_color COLOR_RED = COLOR(RED, YELLOW);
// warning << BOLD << COLOR_RED << "FIRE!" << NEXT_LINE;
//
Curses_pos& Curses_pos::operator<<(const Curses_color& cc)
{
    wattron(active, COLOR_PAIR(cc()));
    color_ids.push_back(cc());
    return *this;
}

// Creates a color object that can be used in the character stream
// from the curses position object.
//
// EXAMPLE:
// Curses_color COLOR_RED = COLOR(RED, BLACK);
//
Curses_color COLOR(int foreground, int background)
{
    return Curses_color(foreground, background);
}

// Constructs a color object by using the init_pair curses function
Curses_color::Curses_color(int foreground, int background)
{
    static int i = 1;
    init_pair(i, foreground, background);
    id = i++;
}

// Returns the id of the color so it can be used by curses.
int Curses_color::operator()() const
{
    return id;
}
