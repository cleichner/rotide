#include <rotide/curses.hpp>
#include <rotide/scripting.hpp>

#include <clocale>

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");

    Curses curses;
    Scripting_engine engine(&curses);

    if (engine.good)
        curses.clear();

    char c = '\0';
    bool insert_mode;

    curses.status() << engine.status(); 
    curses.draw_status_bar();
    Curses_pos at = curses.at(0, 0);
    curses.refresh();

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
