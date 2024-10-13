#include "TFTPServer.h"
#include "TFTPUploadHandler.h"
#include "TFTPDownloadHandler.h"
#include <winsock2.h> // Windows sockets
#include <ws2tcpip.h> // For more socket functions (getpeername, inet_ntop)
#include <direct.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>

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
            logConnection(client_socket, true);  // Log connection
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
    // Send welcome message to the client
    std::string welcomeMsg = "You are connected to TuneFTP!\n";
    send(client_socket, welcomeMsg.c_str(), welcomeMsg.size(), 0);

    // Send a prompt message to simulate an FTP command prompt
    std::string prompt = "ftp> ";
    send(client_socket, prompt.c_str(), prompt.size(), 0);

    char buffer[1024];
    while (true) {
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);  // Winsock recv
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::string command(buffer);
            this->processCommand(client_socket, command);  // Process command
            send(client_socket, prompt.c_str(), prompt.size(), 0);  // Show prompt after each command
        }
        else {
            break;  // Exit the loop if the connection is closed or recv fails
        }
    }

    closesocket(client_socket);  // Winsock closesocket
    logConnection(client_socket, false);  // Log disconnection
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

// Helper function to log connection and disconnection events
void TFTPServer::logConnection(SOCKET client_socket, bool isConnected) {
    sockaddr_in client_info;
    int addr_size = sizeof(client_info);
    char client_ip[INET_ADDRSTRLEN];  // Buffer for the client IP address

    if (getpeername(client_socket, (sockaddr*)&client_info, &addr_size) == 0) {
        inet_ntop(AF_INET, &client_info.sin_addr, client_ip, sizeof(client_ip));
    }
    else {
        strcpy(client_ip, "Unknown IP");
    }

    // Get the current time
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    if (isConnected) {
        std::cout << "[" << std::ctime(&now_c) << "] Client connected: " << client_ip << "\n";
    }
    else {
        std::cout << "[" << std::ctime(&now_c) << "] Client disconnected: " << client_ip << "\n";
    }
}