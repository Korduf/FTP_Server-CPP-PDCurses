#include "display.h"
#include "consts.h"


void Display::DisplayAllWindow()
{
    getmaxyx(stdscr, y_max, x_max);
    init_color(COLOR_YELLOW, 900,450,245);
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_RED);
    init_pair(5, COLOR_YELLOW, COLOR_GREEN);
    server_info = newwin(7, x_max, 0, 0);
    server_output = newwin(0, x_max-30, server_info->_maxy, 0);
    server_threads = newwin(0, 0, server_info->_maxy, server_output->_maxx);
    DisplayServerInfo(server_info, "INFO");
    DisplayServerInfo(server_output, "OUTPUT");
    DisplayServerInfo(server_threads, "CONNECTIONS");
    SettingWindow();
    DisplayPadOutput();
    refresh();
    for (WINDOW* i : {server_info, server_output, server_threads}) {
        wrefresh(i);
    }
    prefresh(output_pad, 0, 0, 9, 2, y_max - 3, server_output->_maxx - 5);
}
void Display::DisplayServerInfo(WINDOW* window, std::string text)
{
    wattron(window, COLOR_PAIR(1));
    wborder(window, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
    wprintw(window, text.c_str());
    wattroff(window, COLOR_PAIR(1));
}
void Display::DisplayPadOutput()
{
    output_pad = newpad(y_max-10, server_output->_maxx -3);
    leaveok(output_pad, true);
    scrollok(output_pad, true);
}
void Display::SettingWindow()
{
    wbkgd(server_threads, COLOR_PAIR(1) | A_REVERSE);
    for (int i = 1; i < 6; ++i) {
        mvwprintw(server_threads, 2 * i, 2, "Connection %d :", i);
        wattron(server_threads, COLOR_PAIR(4)); //ÊÐÀÑÍÛÉ ÍÀ×ÈÍÀÅÒÑß
        mvwaddch(server_threads, 2 * i, 18, ACS_DIAMOND);
        wattroff(server_threads, COLOR_PAIR(4)); //ÊÐÀÑÍÛÉ ÇÀÊÀÍ×ÈÂÀÅÒÑß
    }
    wattron(server_info, COLOR_PAIR(3)); //ÊÐÀÑÍÛÉ ÍÀ×ÈÍÀÅÒÑß
    mvwaddch(server_info, 1, 63, ACS_DIAMOND);
    mvwprintw(server_info, 1, 65, "no connections");
    wattroff(server_info, COLOR_PAIR(3)); //ÊÐÀÑÍÛÉ ÇÀÊÀÍ×ÈÂÀÅÒÑß
    wattron(server_info, COLOR_PAIR(1)); //ÎÐÀÍÆÅÂÛÉ ÍÀ×ÈÍÀÅÒÑß
    mvwprintw(server_info, 1, 55, "Status:");
    for (int i = 1; i < 5; i++) {
        mvwprintw(server_info, i, 2, title[i-1].c_str());
    }
    wattroff(server_info, COLOR_PAIR(1)); //ÎÐÀÍÆÅÂÛÉ ÇÀÊÀÍ×ÈÂÀÅÒÑß
}
void Display::PrintPadOutput(std::string text)
{
    waddch(output_pad, ACS_BULLET);
    waddstr(output_pad, text.append("\n").c_str());
    prefresh(output_pad, 0, 0, 9, 2, y_max - 3, server_output->_maxx - 5);
}
void Display::ChangeClientConnection(int number)
{
    if (number >= 5 || number < 0) {
        return;
    }
    for (int i = 1; i < 6; ++i) {
        if (number == i - 1 && connections[number] == false) {
            connections[number] = true;
            wattron(server_threads, COLOR_PAIR(5)); //ÇÅËÅÍÛÉ ÍÀ×ÈÍÀÅÒÑß
            mvwaddch(server_threads, 2 * i, 18, ACS_DIAMOND);
            wattroff(server_threads, COLOR_PAIR(5)); //ÇÅËÅÍÛÉ ÇÀÊÀÍ×ÈÂÀÅÒÑß
        } else if (number == i - 1 && connections[number] == true) {
            connections[number] = false;
            wattron(server_threads, COLOR_PAIR(4)); //ÊÐÀÑÍÛÉ ÍÀ×ÈÍÀÅÒÑß
            mvwaddch(server_threads, 2 * i, 18, ACS_DIAMOND);
            wattroff(server_threads, COLOR_PAIR(4)); //ÊÐÀÑÍÛÉ ÇÀÊÀÍ×ÈÂÀÅÒÑß
        }
    }
    wrefresh(server_threads);
}