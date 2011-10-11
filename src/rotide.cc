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

#include <rotide/curses.hpp>
#include <rotide/scripting.hpp>

#include <clocale>

int main(int argc, char** argv)
{
    char c;
    bool insert_mode;

    setlocale(LC_ALL, "");

    Curses curses;
    curses.refresh();

    Scripting_engine engine(&curses);


    if (!engine.good)  {
        curses.refresh();
        curses.wait();
        return -1;
    }

    curses.clear();
    curses.draw_status_bar();
    curses.status() << engine.status(); 
    curses.refresh();

    //curses.screen_buffer.clear();
    Curses_pos& at = curses.at(0, 0);

    while (curses.get(&c)) {
        insert_mode = engine.insert_mode();

        engine.think();

        if (!engine.insert_mode())
            insert_mode = false;

        if (insert_mode) {
            at << c;

            if (c == '\n') {
                at.row++;
                at.col = 0;
            }
        }

        curses.refresh();
    }
    return 0;
}
