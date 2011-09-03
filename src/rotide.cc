#include <rotide/curses.hpp>

int main(int argc, char** argv)
{
    Curses curses;

    char c = '\0';

    curses.refresh();
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
