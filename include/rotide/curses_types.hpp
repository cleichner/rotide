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

#ifndef ROTIDE_CURSES_TYPES_HPP
#define ROTIDE_CURSES_TYPES_HPP

#include <vector>
#include <sstream>

typedef std::vector<int> Color_list;

struct Curses_color {
    Curses_color(int foreground, int background);
    int operator()() const;
    int id;
};

#define BIT(shift) ((1U) << (shift + 8))
Curses_color COLOR(int foreground, int background);

enum Curses_extended_keys
{
    CTRL_A = 1,
    CTRL_B,
    CTRL_C,
    CTRL_D,
    CTRL_E,
    CTRL_F,
    CTRL_G,
    CTRL_H,
    CTRL_I,
    CTRL_J,
    CTRL_K,
    CTRL_L,
    CTRL_M,
    CTRL_N,
    CTRL_O,
    CTRL_P,
    CTRL_Q,
    CTRL_R,
    CTRL_S,
    CTRL_T,
    CTRL_U,
    CTRL_V,
    CTRL_W,
    CTRL_X,
    CTRL_Y,
    CTRL_Z,
    SUB,
    ESC,
    COLON = 58,
};

enum Key_string_mode {
    KS_PRETTY_PRINT,
    KS_NO_PRETTY_PRINT,
};

inline
std::string KEY_STR(const int key, const Key_string_mode& mode = KS_PRETTY_PRINT)
{
    std::stringstream buf;
    if (key >= CTRL_A && key <= CTRL_Z) {
        buf << "<CTRL+" << (char)(key + 96) << ">";
    } else {
        if (KS_PRETTY_PRINT)
            buf << "<" << (char)key << ">";
        else
            buf << (char)key;
    }

    std::string s = buf.str();
    return s;
}


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
    GRAY        = 8,
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
    CLEAR = BIT(4),
    FOCUS = BIT(5),
    NOFOCUS = BIT(6)
};

#endif // ROTIDE_CURSES_TYPES_HPP
