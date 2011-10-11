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

#ifndef ROTIDE_CURSES_HPP
#define ROTIDE_CURSES_HPP

#include <rotide/curses_types.hpp>

#include <sstream>
#include <string>
#include <vector>
#include <deque>

class Curses;

// Put curses into its own namespace so it doesn't pollute the project
namespace curses_lib {
#include <ncurses.h>
}

// Ideas:
typedef std::vector<char> Buffer_columns;
typedef std::vector<Buffer_columns> Buffer_rows;
class Curses_buffer {
public:

    Buffer_rows buffer;

    Curses_buffer() {
        buffer.reserve(100);
        rows = 0;
    }

    void resize(int row, int col) {
    }

    void insert(int row, int col, char x) {
        if (row > rows) {
            buffer.push_back(Buffer_columns());
            rows = row;
        }

        buffer[row].push_back(x);
    }

    int rows;
};


// The Curses_pos class describes a position on the screen that can
// be written to using the stream-style operators. It requires
// an instance of a curses window which can be provided by a Curses class
// instance.
//
// EXAMPLE:
//  Curses_pos& pos = curses.at(3, 2);
//  pos << "Hello, world!";
//
class Curses_pos {
public:
    Color_list color_ids;
    bool focus;
    int col, row;
    curses_lib::WINDOW* active;
    Curses* instance;

    Curses_pos() : focus(true) { }
    Curses_pos(int row, int col, curses_lib::WINDOW* window, Curses* instance)
        : row(row), col(col), active(window), instance(instance) { }

    // Provides a way of translating basic types to the
    // the screen; uses stringstream for convenience.
    template <class T>
    Curses_pos& operator<<(const T& t)
    {
        std::stringstream ss;
        ss << t;
        print(ss.str());
        return *this;
    }

    // The various extensions to the << operator.
    Curses_pos& operator<<(const Curses_pos& cp);
    Curses_pos& operator<<(const Curses_style& cs);
    Curses_pos& operator<<(const Curses_action& ca);
    Curses_pos& operator<<(const Curses_color& cc);

    // Print raw strings. Used by the << operator in the end.
    void print(const std::string& s);
};

typedef std::vector<curses_lib::WINDOW*> Buffer_list;

// The Curses class makes it easier to interact with the ncurses library.
// The commands are simplified to fit the needs of rotide without causing
// a bunch of headaches. When used in conjunction with Curses_pos it's
// pretty powerful.
class Curses {
public:
    Curses();
    ~Curses();

    // Check if an input works
    bool check_cursor(int mx, int my);

    // Refresh the window
    void refresh();

    // Shutdown the instance
    void shutdown();

    // Wait for input
    void wait();

    // Draw a line at the current position
    void line();

    // Resize event for the instance
    void resize();

    // Clear the screen
    void clear();

    // Get a character from the input
    bool get(char* c);

    // Draw a status bar
    void draw_status_bar();

    // Inserts a character into the screen buffer
    void insert(const int row, const int col, const char c);

    // Create a curses position instance at the given x, y for input
    Curses_pos& at(const int row, const int col);

    // Get a curses position instance for the status
    Curses_pos& status();

    // last pressed key
    int last_key;

    Curses_pos pos, spos;
    curses_lib::WINDOW* touched_window;
    curses_lib::WINDOW* active_window;
    curses_lib::WINDOW* status_window;

    Buffer_list buffers;
    Curses_buffer screen_buffer;
};

#endif // ROTIDE_CURSES_HPP
