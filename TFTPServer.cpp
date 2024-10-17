#include "TFTPServer.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <direct.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")

TFTPServer::TFTPServer(int port) : tuneFTP_port(port), tuneFTP_socket(INVALID_SOCKET), data_socket(INVALID_SOCKET) {}

TFTPServer::~TFTPServer() {
    stop();
}

void TFTPServer::configureServer() {
    std::cout << "=== Configure TuneFTP Server ===" << std::endl;
    std::cout << "Enter admin username: ";
    std::getline(std::cin, adminUsername);

    std::cout << "Enter admin password: ";
    std::getline(std::cin, adminPassword);

    std::cout << "Enter directory path for client access: ";
    std::getline(std::cin, accessDirectory);

    if (_chdir(accessDirectory.c_str()) != 0) {
        std::cerr << "Error: Directory does not exist." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server configured successfully!\n\n";
}

void TFTPServer::start() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << "\n";
        return;
    }

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
            std::thread clientThread(&TFTPServer::handleClient, this, client_socket);
            clientThread.detach(); // Handle each client in its own thread
        }
    }
}

void TFTPServer::stop() {
    if (tuneFTP_socket != INVALID_SOCKET) {
        closesocket(tuneFTP_socket);
        WSACleanup();
    }
}

void TFTPServer::handleClient(SOCKET client_socket) {
    sendResponse(client_socket, "220 Welcome to TuneFTP!\r\n");
    handleCommands(client_socket);
    closesocket(client_socket);  // Close control connection after the session ends
}

void TFTPServer::handleCommands(SOCKET client_socket) {
    char buffer[1024];
    std::string username;
    bool loggedIn = false;

    while (true) {
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) break; // Exit on connection close

        buffer[bytes_read] = '\0';
        std::string command(buffer);

        if (command.find("USER") == 0) {
            username = command.substr(5);
            sendResponse(client_socket, "331 Username OK, need password.\r\n");
        }
        else if (command.find("PASS") == 0) {
            std::string password = command.substr(5);
            if (username == adminUsername && password == adminPassword) {
                sendResponse(client_socket, "230 User logged in, proceed.\r\n");
                loggedIn = true;
            }
            else {
                sendResponse(client_socket, "530 Login incorrect.\r\n");
                return;  // Disconnect on failed login
            }
        }
        else if (command.find("LIST") == 0) {
            if (loggedIn) handleLIST(client_socket);
        }
        else if (command.find("RETR") == 0) {
            if (loggedIn) handleRETR(client_socket, command.substr(5));
        }
        else if (command.find("STOR") == 0) {
            if (loggedIn) handleSTOR(client_socket, command.substr(5));
        }
        else if (command.find("QUIT") == 0) {
            handleQUIT(client_socket);
            break;
        }
        else if (command.find("PORT") == 0) {
            handlePORT(client_socket, command.substr(5));
        }
        else if (command.find("PASV") == 0) {
            handlePASV(client_socket);
        }
        else {
            sendResponse(client_socket, "500 Unknown command.\r\n");
        }
    }
}

void TFTPServer::handlePORT(SOCKET client_socket, const std::string& params) {
    // Parse PORT command parameters and establish an active data connection
    int h1, h2, h3, h4, p1, p2;
    sscanf(params.c_str(), "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2);

    std::string ip = std::to_string(h1) + "." + std::to_string(h2) + "." +
        std::to_string(h3) + "." + std::to_string(h4);
    int port = (p1 * 256) + p2;

    sockaddr_in data_addr{};
    data_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &data_addr.sin_addr);
    data_addr.sin_port = htons(port);

    data_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(data_socket, (sockaddr*)&data_addr, sizeof(data_addr)) == SOCKET_ERROR) {
        sendResponse(client_socket, "425 Can't open data connection.\r\n");
        return;
    }

    sendResponse(client_socket, "200 PORT command successful.\r\n");
}

void TFTPServer::handlePASV(SOCKET client_socket) {
    sockaddr_in data_addr{};
    data_socket = socket(AF_INET, SOCK_STREAM, 0);
    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = INADDR_ANY;
    data_addr.sin_port = 0;

    bind(data_socket, (sockaddr*)&data_addr, sizeof(data_addr));

    int addr_len = sizeof(data_addr);
    getsockname(data_socket, (sockaddr*)&data_addr, &addr_len);
    listen(data_socket, 1);

    unsigned short port = ntohs(data_addr.sin_port);
    unsigned int ip;
    getsockname(client_socket, (sockaddr*)&data_addr, &addr_len);
    ip = ntohl(data_addr.sin_addr.s_addr);

    std::string pasv_response = "227 Entering Passive Mode (";
    pasv_response += std::to_string((ip >> 24) & 0xFF) + ",";
    pasv_response += std::to_string((ip >> 16) & 0xFF) + ",";
    pasv_response += std::to_string((ip >> 8) & 0xFF) + ",";
    pasv_response += std::to_string(ip & 0xFF) + ",";
    pasv_response += std::to_string(port / 256) + ",";
    pasv_response += std::to_string(port % 256) + ").\r\n";

    sendResponse(client_socket, pasv_response);
}

void TFTPServer::handleLIST(SOCKET client_socket) {
    SOCKET data_conn = acceptDataConnection(client_socket);
    if (data_conn == INVALID_SOCKET) {
        sendResponse(client_socket, "425 Can't open data connection.\r\n");
        return;
    }

    sendResponse(client_socket, "150 Opening ASCII mode data connection for file list.\r\n");

    std::string dirList = "-rw-r--r-- 1 user group 4096 Jan 1 00:00 file1.txt\r\n";
    send(data_conn, dirList.c_str(), dirList.size(), 0);

    sendResponse(client_socket, "226 Directory send OK.\r\n");
    closesocket(data_conn);
}

void TFTPServer::handleRETR(SOCKET client_socket, const std::string& filename) {
    SOCKET data_conn = acceptDataConnection(client_socket);
    if (data_conn == INVALID_SOCKET) {
        sendResponse(client_socket, "425 Can't open data connection.\r\n");
        return;
    }

    std::ifstream file(accessDirectory + "\\" + filename, std::ios::binary);
    if (!file) {
        sendResponse(client_socket, "550 File not found.\r\n");
        closesocket(data_conn);
        return;
    }

    sendResponse(client_socket, "150 Opening binary mode data connection for " + filename + "\r\n");

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer))) {
        send(data_conn, buffer, file.gcount(), 0);
    }
    sendResponse(client_socket, "226 Transfer complete.\r\n");
    closesocket(data_conn);
}

void TFTPServer::handleSTOR(SOCKET client_socket, const std::string& filename) {
    SOCKET data_conn = acceptDataConnection(client_socket);
    if (data_conn == INVALID_SOCKET) {
        sendResponse(client_socket, "425 Can't open data connection.\r\n");
        return;
    }

    std::ofstream file(accessDirectory + "\\" + filename, std::ios::binary);
    if (!file) {
        sendResponse(client_socket, "550 Failed to create file.\r\n");
        closesocket(data_conn);
        return;
    }

    sendResponse(client_socket, "150 Opening binary mode data connection for " + filename + "\r\n");

    char buffer[1024];
    int bytes_read;
    while ((bytes_read = recv(data_conn, buffer, sizeof(buffer), 0)) > 0) {
        file.write(buffer, bytes_read);
    }
    sendResponse(client_socket, "226 Transfer complete.\r\n");
    closesocket(data_conn);
}

void TFTPServer::handleQUIT(SOCKET client_socket) {
    sendResponse(client_socket, "221 Goodbye.\r\n");
}

void TFTPServer::sendResponse(SOCKET client_socket, const std::string& response) {
    send(client_socket, response.c_str(), response.size(), 0);
}

SOCKET TFTPServer::acceptDataConnection(SOCKET client_socket) {
    if (data_socket == INVALID_SOCKET) {
        sendResponse(client_socket, "425 No data connection.\r\n");
        return INVALID_SOCKET;
    }
    sockaddr_in data_addr{};
    int addr_len = sizeof(data_addr);
    SOCKET data_conn = accept(data_socket, (sockaddr*)&data_addr, &addr_len);
    closesocket(data_socket); // Close the listening socket
    data_socket = INVALID_SOCKET;
    return data_conn;
}

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