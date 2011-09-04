#include <rotide/curses.hpp>
#include <rotide/scripting.hpp>

#include <clocale>

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");

    Curses curses;
    curses.refresh();

    Scripting_engine engine(&curses);

    char c = '\0';

    Curses_pos at = curses.at(0, 0);

    while (curses.get(&c)) {
        at << c;

        if (c == '\n') {
            at.row++;
            at.col = 0;
        }
        
    }
    return 0;
}
