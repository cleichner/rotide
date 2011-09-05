#include "rotide/curses.hpp"
#include <csignal>
#include <cassert>

using namespace curses_lib;

namespace {
typedef std::vector<Curses*> Curses_instances;
Curses_instances instances;
}

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

    initscr();
    start_color();
    raw();
    keypad(stdscr, TRUE);

    noecho();

    // Create a buffer
    getmaxyx(stdscr, row, col);
    active = newwin(row, col, 0, 0);
    box(active, ACS_VLINE, ACS_HLINE);
    buffers.push_back(active);

    // Add to the instances for signals
    instances.push_back(this);
} 

Curses::~Curses()
{
    shutdown();
}

void Curses::resize()
{
    for (Buffer_list::const_iterator cit = buffers.begin(),
            end = buffers.end();
            cit != end;
            ++cit)
    {
    }
}

void Curses::shutdown()
{
    endwin();
}

void Curses::refresh()
{
    ::refresh();
    wrefresh(active);
}

void Curses::line()
{
    int row, col;
    getmaxyx(stdscr, row, col);
    whline(active, ACS_HLINE, row);
}

void Curses::wait()
{
    getch();
}

bool Curses::get(char* s)
{
    *s = getch();
    if (*s == 3)
        return false;
    return true;
}


Curses_pos& Curses::at(const int x, const int y)
{
    pos.col = y;
    pos.row = x;
    pos.active = active;
    pos.col = pos.col == 0 ? 1 : pos.col;
    pos.row = pos.row == 0 ? 1 : pos.row;
    return pos;
}

void Curses_pos::print(const std::string& s)
{
    mvwprintw(active, row, col, "%s", s.c_str());
    col += s.size();
}

Curses_pos& Curses_pos::operator<<(const Curses_pos& cp)
{
    col = cp.col;
    row = cp.row;
    active = cp.active;
    return *this;
}

Curses_pos& Curses_pos::operator<<(const Curses_action& ca)
{
    long int action = (long int)ca;
    if (action & NEXT_LINE) {
        col = 1;
        row++;
    } else if (action & RESET) {
        for (Color_list::const_iterator 
                cit = color_ids.begin(),
                end = color_ids.end();
                cit != end;
                ++cit)
        {
            wattroff(active, *cit);
        }

        wattrset(active, NORMAL);
    } else if (action & HLINE) {
        int mcol, mrow;
        getmaxyx(stdscr, mrow, mcol);
        mvwhline(active, row, 1, ACS_HLINE, mcol - 2);
    }

    return *this;
}

Curses_pos& Curses_pos::operator<<(const Curses_style& cs)
{
    long int style = (long int)cs;
    wattron(active, style);
    return *this;
}

Curses_pos& Curses_pos::operator<<(const Curses_color& cc)
{
    wattron(active, COLOR_PAIR(cc()));
    color_ids.push_back(cc());
    return *this;
}

Curses_color COLOR(int foreground, int background)
{
    return Curses_color(foreground, background);
}

Curses_color::Curses_color(int foreground, int background)
{
    static int i = 1;
    init_pair(i, foreground, background);
    id = i++;
}

int Curses_color::operator()() const
{
    return id;
}
