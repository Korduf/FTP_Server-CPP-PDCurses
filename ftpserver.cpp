#include <fstream>
#include "ftpserver.h"

DualParam clientParam;
ClientStruct clientStruct[5];
int connection = 0;

FTPServer::FTPServer(Display &display_t)
{
    WSAData wsa_data;
    display_ftp = display_t;
    int err = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if ( err != 0 ) {
        display_ftp.PrintPadOutput("ERROR: WinSock version initialization - " + std::to_string(WSAGetLastError()));
        abort();
    }
    else
        display_ftp.PrintPadOutput("SUCCESS: WinSock initialization is OK");
}

int FTPServer::CreateSocket()
{
    socket_server = socket(AF_INET, SOCK_STREAM, 0);
    in_addr ip_for_socket;
    if (socket_server == INVALID_SOCKET) {
        display_ftp.PrintPadOutput("ERROR: Create socket");
    }
    int err_socket = inet_pton(AF_INET, "127.0.0.1", &ip_for_socket);
    if (err_socket <= 0) {
        display_ftp.PrintPadOutput("ERROR: Convert IP to special numeric format");
    }
    ZeroMemory(&server_address, sizeof(server_address));
    server_address.sin_addr = ip_for_socket;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(21);
    err_socket = bind(socket_server, (sockaddr*)&server_address, sizeof(server_address));
    if (err_socket != 0) {
        display_ftp.PrintPadOutput("ERROR: Fail binding to server info - " + std::to_string(WSAGetLastError()));
        closesocket(socket_server);
        WSACleanup();
        return 1;
    }
    else
        display_ftp.PrintPadOutput("SUCCESS: Binding to server info is ok");
}

int FTPServer::StartListener()
{
    int err_socket = listen(socket_server, SOMAXCONN);
    if (err_socket != 0) {
        display_ftp.PrintPadOutput("ERROR: Can't start listen to - " + std::to_string(WSAGetLastError()));
        closesocket(socket_server);
        WSACleanup();
        return 1;
    }
    else
        display_ftp.PrintPadOutput("SUCCESS: Start listening...");
}

bool FTPServer::User(ClientStruct& client, char* args)
{
    strcpy(client.username, args);
    send(client.client_socket, "331 Please, specify the password", sizeof("331 Please, specify the password"), 0);
    return true;
}

bool FTPServer::Password(ClientStruct& client, char* args)
{
    strcpy(client.password, args);
    char *path = NULL;
    size_t size;
    path = getcwd(path, size);
    std::string str_path(path);
    //std::ifstream in("C:\\Users\\kiril\\CLionProjects\\ftp-server\\users.txt");
    std::ifstream in(str_path + "\\" + "users.txt");
    if (in.is_open()) {
        std::string data;
        while (std::getline(in, data)) {
            char temp_username[128], temp_password[128];
            sscanf(data.c_str(), "%s %s", temp_username, temp_password);
            if (strcmp(temp_username, client.username) == 0 && strcmp(temp_password, client.password) == 0) {
                send(client.client_socket, "230 Success login", sizeof("230 Success login"), 0);
                break;
            }
        }
    }
    in.close();
}

void FTPServer::StartServer()
{
    CreateSocket();
    StartListener();
}

void FTPServer::AcceptNewConnection()
{
    for (int i = 0; i < 5; ++i) {
        if (!clientStruct[i].connected) {
            if (AcceptConnection(clientStruct[i]) == true) {
                clientStruct[i].runner = true;
                clientStruct[i].connection_id = i;
                connection++;
                clientParam.client_param = clientStruct[i];
                display_ftp.ChangeClientConnection(i);
                clientParam.display_param = &display_ftp;
                CreateThread(0, 0, Socket_Thread,
                             (LPVOID) &clientParam, 0, 0);
            }
        }
    }
}

DWORD FTPServer::Socket_Thread(LPVOID param)
{
    //ClientStruct client = *(ClientStruct*)param;
    DualParam dualParam = *(DualParam*)param;
    ClientStruct &client = dualParam.client_param;
    //--------------------------
    dualParam.display_param->PrintPadOutput("SUCCESS: Connection to a client established");
    send(client.client_socket, "220 You connect to FTP Server", sizeof("220 You connect to FTP Server"), 0);
    //--------------------------
    while (client.runner) {
        client.runner = FTPServer::CheckCommand(client, dualParam.display_param);
    }
    client.connected = false;
    client.runner = false;
    closesocket(client.data_socket);
    closesocket(client.client_socket);
    return 1;
}

bool FTPServer::CheckCommand(ClientStruct& client, Display* display)
{
    std::vector<char> buffer(1024);
    std::string data;
    std::string cmd, args;
    int err_socket = recv(client.client_socket, &buffer[0], buffer.size(), 0);
    data.append(buffer.cbegin(), buffer.cend());
    for (char c: "\r\n\000") {
        data.erase(std::remove(data.begin(), data.end(), c), data.end());
    }
    if (err_socket <= 0) {
        connection--;
        display->ChangeClientConnection(client.connection_id);
        closesocket(client.client_socket);
        return false;
    }
    display->PrintPadOutput("SUCCESS: Received: " + data);
    auto pos = data.find(" ");
    cmd = data.substr(0, pos);
    args = data.substr(pos + 1);
    (args[0] == 0 ? std::cout << "CMD: " << cmd : std::cout << "CMD: " << cmd << ", ARGS: " << args) << std::endl;
    if (cmd == "USER") CommandSuccess(User(client, (char*)args.c_str()), (char*)cmd.c_str(), display);
    if (cmd == "PASS") CommandSuccess(Password(client, (char*)args.c_str()), (char*)cmd.c_str(), display);
    if (cmd == "PUT") CommandSuccess(UploadFile(client, (char*)args.c_str()), (char*)cmd.c_str(), display);
    if (cmd == "GET") CommandSuccess(DownloadFile(client, (char*)args.c_str()), (char*)cmd.c_str(), display);
    if (cmd == "SYST") CommandSuccess(System(client), (char*)cmd.c_str(), display);
    if (cmd == "FEAT") CommandSuccess(FeatureList(client), (char*)cmd.c_str(), display);
    if (cmd == "PWD") CommandSuccess(CurrentDir(client), (char*)cmd.c_str(), display);
    if (cmd == "CWD") CommandSuccess(ChangeDirectory(client, (char*)args.c_str()), (char*)cmd.c_str(), display);
    if (cmd == "NOOP") CommandSuccess(ServerActiveResponce(client), (char*)cmd.c_str(), display);
    if (cmd == "LIST") CommandSuccess(ListDirectories(client, display), (char*)cmd.c_str(), display);
    if (cmd == "TYPE") CommandSuccess(TransferType(client), (char*)cmd.c_str(), display);
    if (cmd == "DELE") CommandSuccess(DeleteServerFile(client, (char*)args.c_str(), display), (char*)cmd.c_str(), display);
    if (cmd == "PORT") CommandSuccess(PortCommand(client, (char*)args.c_str(), display), (char*)cmd.c_str(), display);
    if (cmd == "RETR") CommandSuccess(ReadyToDownload(client, (char*)args.c_str(), display), (char*)cmd.c_str(), display);
    if (cmd == "STOR") CommandSuccess(ReadyToUpload(client, (char*)args.c_str(), display), (char*)cmd.c_str(), display);
}

bool FTPServer::CommandSuccess(bool flag, char* cmd, Display* display)
{
    std::string cmd_str(cmd);
    if (flag) {
        display->PrintPadOutput("Command: " + cmd_str + " complete");
    }
    else
        display->PrintPadOutput("Fail to complete command");
}

bool FTPServer::SendResponce(ClientStruct& client, std::string code, std::string text)
{
    std::stringstream string_stream;
    string_stream << code << text <<"\n";
    send(client.client_socket, string_stream.str().c_str(),strlen(string_stream.str().c_str()), 0);
}

bool FTPServer::AcceptConnection(ClientStruct& client)
{
    client.data_socket = -1;
    client.client_address_size = sizeof(sockaddr);
    client.client_socket = accept(socket_server, (sockaddr*)&client.client_address, &client.client_address_size);
    if (client.client_socket != 0 && client.client_socket != SOCKET_ERROR) {
        client.connected = true;
        return true;
    }
    return false;
}

bool FTPServer::System(ClientStruct& client)
{
    send(client.client_socket, "215 WINDOWS MAYBE", sizeof("215 WINDOWS MAYBE"), 0);
}

bool FTPServer::FeatureList(ClientStruct& client)
{
    send(client.client_socket, "211 Features: None", sizeof("211 Features: None"),0);
}

bool FTPServer::CurrentDir(ClientStruct& client)
{
    char *path = NULL;
    size_t size;
    path = getcwd(path, size);
    client.current_dir = path;
    std::stringstream string_stream;
    string_stream << "257 \""<< path <<"\" \n";
    send(client.client_socket, string_stream.str().c_str(), strlen(string_stream.str().c_str()), 0);
    return true;
}

bool FTPServer::TransferType(ClientStruct& client)
{
    //надо еще A добавить
    std::stringstream string_stream;
    string_stream << "200 "<< "TYPE is now 8-bit binary" <<"\n";
    send(client.client_socket, string_stream.str().c_str(),
         strlen(string_stream.str().c_str()), 0);
    return true;
}

bool FTPServer::PortCommand(ClientStruct &client, char* args, Display* display)
{
    //-----------------------------------------сократить
    std::stringstream string_stream;
    int address[4];
    int port[2];
    in_addr ip_for_info_thread;
    sscanf(args, "%d,%d,%d,%d,%d,%d", &address[0], &address[1],
           &address[2], &address[3], &port[0], &port[1]);
    std::string ip_str = std::to_string(address[0]) + "." + std::to_string(address[1])
            + "." + std::to_string(address[2]) + "." + std::to_string(address[3]);
    //uint32_t tcp_address = address[0] << 24 | address[1] << 16 | address[2] << 8 | address[3];
    //int err_socket = inet_pton(AF_INET, "127.0.0.1", &ip_for_info_thread);
    int err_socket = inet_pton(AF_INET, ip_str.c_str(), &ip_for_info_thread);
    //-----------------------------------------
    if (err_socket <= 0) {
        display->PrintPadOutput("ERROR: Can't interpret parameters");
        string_stream << "501 " << "Can't interpret parameters" << "\n";
    } else {
        /*client.data_socket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in server_data_address;
        server_data_address.sin_addr = ip_for_info_thread;
        server_data_address.sin_family = AF_INET;
        server_data_address.sin_port = htons(20);
        bind(client.data_socket, (sockaddr*)&server_data_address, sizeof(server_data_address));*/
        err_socket = DataConnection(client, display);
        if (client.data_socket < 0) {
            display->PrintPadOutput("ERROR: Attempt to initialize socket");
            return false;
        }
        /*client.information_address.sin_port = htons((port[0] * 256) + port[1]);*/
        client.information_address.sin_port = htons((port[0] << 8) | port[1]);
        client.information_address.sin_addr = ip_for_info_thread;
        client.information_address.sin_family = AF_INET;
        //------------------------------------ из функции достал
        if (connect(client.data_socket, (struct sockaddr*) &client.information_address,
                    sizeof(client.information_address)) != 0) {
            display->PrintPadOutput("ERROR: Attempt to connect data thread");
            return false;
        }
        //------------------------------------
        if (err_socket == 1) {
            string_stream << "200 " << "PORT command successful " << "\n";
        } else { //?
            display->PrintPadOutput("ERROR: Port can't open");
            string_stream << "425 " << "PORT can't open " << "\n";
        }
    }
    send(client.client_socket, string_stream.str().c_str(),
         strlen(string_stream.str().c_str()), 0);
    return true;
}

bool FTPServer::DataConnection(ClientStruct &client, Display* display)
{
    in_addr ip_for_data_socket;
    inet_pton(AF_INET, "127.0.0.1", &ip_for_data_socket);
    client.data_socket = socket(AF_INET, SOCK_STREAM, 0); //Настройка портов для ftp-data
    sockaddr_in server_data_address;
    server_data_address.sin_addr = ip_for_data_socket;
    server_data_address.sin_family = AF_INET;
    server_data_address.sin_port = htons(20);
    bind(client.data_socket, (sockaddr*)&server_data_address, sizeof(server_data_address));
    return true;
}

bool FTPServer::ReadyToDownload(ClientStruct &client, char *args, Display *display)
{
    //Добавить проверку на доступность порта и авторизации
    std::string file_name(args);
    FILE *file;
    char buffer[1024];
    int size = 1024;
    file = fopen((client.current_dir + "\\" + file_name).c_str(), "rb");
    display->PrintPadOutput("SENDING FILE: " + file_name);
    if (file != nullptr) {
        SendResponce(client, "150 ", "Opening BINARY mode data connection");
        while (size != 0) {
            size = fread(buffer, 1, 1024, file);
            send(client.data_socket, buffer, size, MSG_WAITALL);
        }
        shutdown(client.data_socket, SD_SEND);
        close(client.data_socket);
        SendResponce(client, "226 ", "Data transmission OK");
        fclose(file);
    } else {
        close(client.data_socket);
        SendResponce(client, "550 ", "File not found");
        fclose(file);
    }
    return true;
}

bool FTPServer::ReadyToUpload(ClientStruct &client, char *args, Display *display)
{
    std::string file_name(args);
    FILE *file;
    char buffer[1024];
    int size = 1024;
    file = fopen((client.current_dir + "\\" + file_name).c_str(), "wb");
    display->PrintPadOutput("RECV FILE: " + file_name);
    if (file != nullptr) {
        SendResponce(client, "150 ", "Opening BINARY mode data connection");
        while (size != 0) {
            size = recv(client.data_socket, buffer, 1024, MSG_WAITALL);
            fwrite(buffer, 1, 1024, file);
        }
        fclose(file);
        shutdown(client.data_socket, SD_SEND);
        close(client.data_socket);
        SendResponce(client, "226 ", "Data transmission OK");
    } else {
        close(client.data_socket);
        SendResponce(client, "550 ", "File not available");
        fclose(file);
    }

    return true;
}

bool FTPServer::ServerActiveResponce(ClientStruct &client)
{
    SendResponce(client, "200 ", "Server is still active");
}

bool FTPServer::DeleteServerFile(ClientStruct &client, char* args, Display* display)
{
    std::string file_name(args);
    display->PrintPadOutput("Command: Delete file with name: " + file_name);
    if (remove(args) == 0) {
        SendResponce(client, "250 ", "Delete operation successful");
        display->PrintPadOutput("SUCCESS: File delete");
        return true;
    } else {
        SendResponce(client, "550 ", "Delete operation error");
        display->PrintPadOutput("ERROR: Something happen");
        return false;
    }
}

bool FTPServer::ListDirectories(ClientStruct& client, Display* display)
{
    DIR* dir;
    struct dirent *file;
    if (client.current_dir.empty()) {
        client.current_dir = "C:\\Users\\kiril\\CLionProjects\\ftp-server";
    }
    dir = opendir(client.current_dir.c_str());
    if (!dir) {
        display->PrintPadOutput("ERROR: Open directories");
        return false;
    }
    SendResponce(client, "150 ", "Opening BINARY mode data connection");
    while ((file = readdir(dir)) != NULL) {
        struct _stat64 file_stat;
        char time_buffer[20];
        _stat64(file->d_name, &file_stat);
        char separate_file[100];
        time_t stat_time = file_stat.st_mtime;
        struct tm time_converted;
        localtime_s(&time_converted, &stat_time);
        strftime(time_buffer, sizeof(time_buffer), "%b %d %H:%M", &time_converted);
        sprintf(separate_file, "%-11s %d %s %14d %s %s\r\n",
                "-rw-r--r--", 1, "owner group", file_stat.st_size, time_buffer, file->d_name);
        std::string str(separate_file);
        send(client.data_socket, str.c_str(), strlen(str.c_str()), 0);
    }
    shutdown(client.data_socket, SD_SEND);
    close(client.data_socket);
    SendResponce(client, "226 ", "Data transmission OK");
    closedir(dir);
    return true;
}

bool FTPServer::ChangeDirectory(ClientStruct &client, char *args)
{
    std::string str_path(args);
    client.current_dir = str_path;
    std::stringstream string_stream;
    string_stream << "250 "<< "Directory change success" <<" \n";
    send(client.client_socket, string_stream.str().c_str(), strlen(string_stream.str().c_str()), 0);
    return true;
}

bool FTPServer::DownloadFile(ClientStruct& client, char *filename) {
    send(client.client_socket, "\nData stream to download established", sizeof(char) * 37, 0);
    char data[64];
    int err_socket = recv(client.client_socket, data, sizeof(data), 0);
    data[err_socket] = 0;
    if (err_socket <= 0) {
        connection--;
        closesocket(client.client_socket);
        client.runner = false;
        return false;
    }
    if (strcmp(data, "ok") == 0) send(client.client_socket, filename, strlen(filename), 0);
    else
        return false;
    return false;
}

bool FTPServer::UploadFile(ClientStruct& client, char *args) {
    return false;
}

FTPServer::~FTPServer() {
    closesocket(socket_server);
    WSACleanup();
}

