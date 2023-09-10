# FTP Server
This repository contains the code for FTP (File Transfer Protocol) server. The server allows users to connect via an FTP client and perform various file transfer operations.To implement the server, the Windows Socket API (winsock) was used. The interface was implemented using the PDCurses library

# Features
Multiple client connections: The server supports multiple client connections concurrently. Each client can connect to the server and perform file transfer operations independently.

Authentication: Users are required to provide a username and password to connect to the server. Only authenticated users can access and transfer files.

File operations: Once connected, clients can perform various FTP operations such as uploading files to the server, downloading files from the server, deleting files, etc.

Directory listings: Clients can request a directory listing of files and directories available on the server.

# Connecting to the Server
To connect to the server, use any FTP client software (e.g., FileZilla, WinSCP, Cyberduck) and provide the following details:

Host: The IP address or hostname of the FTP server(currently works only locally).

Port: The FTP port (default is 21).

Username, Password: The configured in the users.txt file.
