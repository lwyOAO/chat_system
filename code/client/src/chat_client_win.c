#include <ncurses.h>

extern WINDOW *display_win;
extern WINDOW *input_win;

void show_msg(char* name, char* msg)
{
    wprintw(display_win, "%s: %s\n", name, msg);
    wrefresh(display_win);
}