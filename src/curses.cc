#include "rotide/curses.hpp"
#include <ncurses.h>
#include <iostream>

Curses::Curses()
{
    initscr();
    start_color();
    raw();
    keypad(stdscr, TRUE);
    noecho();
}

Curses::~Curses()
{
    shutdown();
}

void Curses::shutdown()
{
    endwin();
}

void Curses::refresh()
{
    ::refresh();
}

void Curses::wait()
{
    getch();
}


Curses_pos& Curses::at(const int x, const int y)
{
    pos.col = y;
    pos.row = x;
    return pos;
}

void Curses_pos::print(const std::string& s)
{
    mvprintw(row, col, "%s", s.c_str());
    col += s.size();
}

Curses_pos& Curses_pos::operator<<(const Curses_pos& cp)
{
    col = cp.col;
    row = cp.row;
    return *this;
}

Curses_pos& Curses_pos::operator<<(const Curses_style& cs)
{
    long int style = (long int)cs;
    if (style & RESET || style & NORMAL) {
        for (Color_list::const_iterator 
                cit = color_ids.begin(),
                end = color_ids.end();
                cit != end;
                ++cit)
        {
            attroff(*cit);
        }

        attrset(NORMAL);
        style &= ~RESET;
    }

    attron(style);
    return *this;
}

Curses_pos& Curses_pos::operator<<(const Curses_color& cc)
{
    attron(COLOR_PAIR(cc()));
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
