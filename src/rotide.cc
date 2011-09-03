#include <rotide/curses.hpp>

int main(int argc, char** argv)
{
    Curses curses;
    curses.refresh();
    curses.at(0, 0) << COLOR(BLACK, WHITE)
                    << "Name:"
                    << RESET 
                    << BOLD
                    << " Justin Bruce Van Horne";
    curses.at(50, 50)
                    << "Hello, world!";
    curses.wait();
    return 0;
}
