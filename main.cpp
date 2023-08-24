//#include "display.h"
#include "ftpserver.h"

int main(int argc, char *argv[]) {
    //-----------Initialize curses-----------
    initscr();
    cbreak();
    if (!has_colors() && COLORS != 256 && !can_change_color()) {
        exit(-1);
    }
    start_color();
    Display display;
    display.DisplayAllWindow();
    int running = true;
    //---------------------------------------
    FTPServer server(display);
    server.StartServer();
    while(running)
    {
        server.AcceptNewConnection();
    }
    //-----------End curses-----------
    getch();
    endwin();
    return 0;
}
