#ifndef FTP_SERVER_DISPLAY_H
#define FTP_SERVER_DISPLAY_H
#include <curses.h>
#include <iostream>


class Display {
private:
    int y_max, x_max; //maximum and minimum x, y windows
    //windows..
    WINDOW *server_info; //a window in which information about the operation of the application will be displayed
    WINDOW *server_output; //a window in which the output of requests and responses to / from the client will be displayed
    WINDOW *server_threads; //a window that will display active connections to the server
    //pad..
    WINDOW *output_pad;
    //connection status..
    bool connections[5] = {false};

public:
    //window render..
    void DisplayAllWindow(); //(old)"so far" the main function for rendering the interface and interacting with the backend of the server
    void DisplayServerInfo(WINDOW* window, std::string text); //drawing a design for each of the windows
    void SettingWindow(); //drawing small details and some window settings
    //pad render..
    void DisplayPadOutput(); //creating a panel to use in output
    //pad print message..
    void PrintPadOutput(std::string text); //a function for displaying text on a panel with a specific design
    //window change connection..
    void ChangeClientConnection(int number); //function to change the status of connections (the number in the parameters means the number of the connection)
};

#endif
