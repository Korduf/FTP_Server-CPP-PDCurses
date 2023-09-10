//#include "display.h"
#include "ftpserver.h"
#define _WIN32_WINNT 0x0500
#include <windows.h>

int main(int argc, char *argv[]) {
    HWND consoleWindow = GetConsoleWindow(); //static terminal size
    SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
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
