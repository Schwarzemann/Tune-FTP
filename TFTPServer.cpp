#include "TFTPServer.h"
#include "TFTPUploadHandler.h"
#include "TFTPDownloadHandler.h"
#include <winsock2.h> // Windows sockets
#include <ws2tcpip.h> // For more socket functions (getaddrinfo)
#include <direct.h>
#include <windows.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib") // Link Winsock library

TFTPServer::TFTPServer(int port) : tuneFTP_port(port), tuneFTP_socket(INVALID_SOCKET) {}

TFTPServer::~TFTPServer() {
    stop();
}

void TFTPServer::start() {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << "\n";
        return;
    }

    // Create a socket for the server
    tuneFTP_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tuneFTP_socket == INVALID_SOCKET) {
        std::cerr << "Error: Failed to create socket.\n";
        WSACleanup();
        return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(tuneFTP_port);

    if (bind(tuneFTP_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Error: Bind failed.\n";
        closesocket(tuneFTP_socket);
        WSACleanup();
        return;
    }

    listen(tuneFTP_socket, SOMAXCONN);
    std::cout << "TuneFTP Server started on port " << tuneFTP_port << "\n";

    while (true) {
        SOCKET client_socket = accept(tuneFTP_socket, nullptr, nullptr);
        if (client_socket != INVALID_SOCKET) {
            handleConnection(client_socket);
        }
    }
}

void TFTPServer::stop() {
    if (tuneFTP_socket != INVALID_SOCKET) {
        closesocket(tuneFTP_socket);
        WSACleanup();
    }
}

void TFTPServer::handleConnection(SOCKET client_socket) {
    char buffer[1024];
    int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);  // Winsock recv
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        std::string command(buffer);
        this->processCommand(client_socket, command);  // Call with 'this'
    }
    closesocket(client_socket);  // Winsock closesocket
}

void TFTPServer::processCommand(SOCKET client_socket, const std::string& command) {
    // Implementation of the processCommand function
    if (command.find("CD") == 0) {
        switchDirectory(client_socket, command.substr(3));
    }
    else if (command.find("GET") == 0) {
        TFTPDownloadHandler::handleDownload(client_socket, command.substr(4));
    }
    else if (command.find("PUT") == 0) {
        TFTPUploadHandler::handleUpload(client_socket, command.substr(4));
    }
    else {
        std::string response = "Invalid Command\n";
        send(client_socket, response.c_str(), response.size(), 0);  // Use send in Windows
    }
}

void TFTPServer::switchDirectory(SOCKET client_socket, const std::string& path) {
    if (_chdir(path.c_str()) == 0) {
        std::string response = "Directory changed to " + path + "\n";
        send(client_socket, response.c_str(), response.size(), 0);
    }
    else {
        std::string response = "Failed to change directory\n";
        send(client_socket, response.c_str(), response.size(), 0);
    }
}