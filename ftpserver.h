#ifndef FTP_SERVER_FTPSERVER_H
#define FTP_SERVER_FTPSERVER_H
#include <iostream>
#include <winsock.h>
#include <WinSock2.h> //Библиотека для работы с сокетами
#include <WS2tcpip.h> //Библиотека для работы с протоколами
#include <dir.h>
#include <dirent.h>
#include <sstream>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include "display.h"
#pragma comment(lib, "Ws2_32.lib")

struct ClientStruct{
    int connection_id;
    SOCKET client_socket;
    SOCKET data_socket;
    char username[128];
    char password[128];
    sockaddr_in information_address;
    sockaddr_in client_address;
    int client_address_size;
    bool connected;
    bool runner;
    std::string current_dir;
};

struct DualParam{
    ClientStruct client_param;
    Display *display_param;
};

struct FilePacket{

};

typedef struct {
    int header;
    char buffer[512];
    int footer;
} Packet;


class FTPServer {
public:
    explicit FTPServer(Display &display_t);
    ~FTPServer();

    int CreateSocket();
    int StartListener();
    int StopSocket();

    void StartServer();
    void AcceptNewConnection();
    bool AcceptConnection(ClientStruct& client);
    static DWORD WINAPI Socket_Thread(LPVOID param);
    static bool CheckCommand(ClientStruct& client, Display* display);
    //Common command-----------------------------------------------
    static bool DownloadFile(ClientStruct& client, char* args);
    static bool UploadFile(ClientStruct& client, char* args);
    static bool ChangeDirectory(ClientStruct& client, char* args);
    static bool System(ClientStruct& client);
    static bool FeatureList(ClientStruct& client);
    static bool CurrentDir(ClientStruct& client);
    static bool ListDirectories(ClientStruct& client, Display* display);
    static bool TransferType(ClientStruct& client);
    static bool PortCommand(ClientStruct& client, char* args, Display* display);
    static bool ServerActiveResponce(ClientStruct& client); //noop
    static bool DeleteServerFile(ClientStruct& client, char* args, Display* display);
    //Additional functions-----------------------------------------
    static bool DataConnection(ClientStruct& client, Display* display);
    //Data transfer command----------------------------------------
    static bool ReadyToDownload(ClientStruct& client, char* args, Display* display); //Download from server
    static bool ReadyToUpload(ClientStruct& client, char* args, Display* display); //Upload to server
    //Login Command------------------------------------------------
    static bool User(ClientStruct& client, char* args);
    static bool Password(ClientStruct& client, char* args);
    //-------------------------------------------------------------
    static bool CommandSuccess(bool flag, char* cmd, Display* display);
    static bool SendResponce(ClientStruct& client, std::string code, std::string text);


    Display display_ftp;
    SOCKET socket_server;
    sockaddr_in server_address;
};


#endif
