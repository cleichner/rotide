#include <rotide/curses.hpp>
#include <rotide/scripting.hpp>

#include <clocale>

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");

    Curses curses;
    Scripting_engine engine(&curses);

    Curses_pos at = curses.at(0, 0);
    curses.clear();
    curses.refresh();

    char c = '\0';

    while (curses.get(&c)) {
        at << (int)c;

        if (c == '\n') {
            at.row++;
            at.col = 0;
        }

        engine.think();
        curses.refresh();
    }
    return 0;
}
