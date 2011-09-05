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
//
// Abide by the style guide: http://goo.gl/WkJ9C

#ifndef ROTIDE_CURSES_HPP
#define ROTIDE_CURSES_HPP

#include <sstream>
#include <string>
#include <vector>

namespace curses_lib {
#include <ncurses.h>
}

typedef std::vector<int> Color_list;

struct Curses_color {
    Curses_color(int foreground, int background);
    int operator()() const;
    int id;
};

#define BIT(shift) ((1U) << (shift + 8))
Curses_color COLOR(int foreground, int background);

enum Curses_style
{
    BLACK       = 0,
    RED         = 1,
    GREEN       = 2,
    YELLOW      = 3,
    BLUE        = 4,
    MAGENTA     = 5,
    CYAN        = 6,
    WHITE       = 7,
    NORMAL      = 0,
    ATTRIBUTES  = ~(1U - 1U),
    STANDOUT    = BIT(8),
    UNDERLINE   = BIT(9),
    REVERSE     = BIT(10),
    BLINK       = BIT(11),
    DIM         = BIT(12),
    BOLD        = BIT(13),
    ALTCHARSET  = BIT(14),
    INVIS       = BIT(15),
    PROTECT     = BIT(16),
    HORIZONTAL  = BIT(17),
    LEFT        = BIT(18),
    LOW         = BIT(19),
    RIGHT       = BIT(20),
    TOP         = BIT(21),
    VERTICAL    = BIT(22),
};

enum Curses_action
{
    NEXT_LINE = BIT(1),
    RESET = BIT(2),
    HLINE = BIT(3),
};

class Curses_pos {
public:
    Color_list color_ids;
    int col, row;
    curses_lib::WINDOW* active;

    Curses_pos() : col(0), row(0) { }

    template <class T>
    Curses_pos& operator<<(const T& t)
    {
        std::stringstream ss;
        ss << t;
        print(ss.str());
        return *this;
    }

    Curses_pos& operator<<(const Curses_pos& cp);
    Curses_pos& operator<<(const Curses_style& cs);
    Curses_pos& operator<<(const Curses_action& ca);
    Curses_pos& operator<<(const Curses_color& cc);

    void print(const std::string& s);
};

typedef std::vector<curses_lib::WINDOW*> Buffer_list;

class Curses {
public:
    Curses();
    ~Curses();
    void refresh();
    void shutdown();
    void wait();
    void line();
    void resize();
    bool get(char* c);
    Curses_pos& at(const int row, const int col);
private:
    Curses_pos pos;
    curses_lib::WINDOW* active;
    Buffer_list buffers;
};

#endif // ROTIDE_CURSES_HPP
