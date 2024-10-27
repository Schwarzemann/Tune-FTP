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
    adminUsername.erase(adminUsername.find_last_not_of(" \n\r\t") + 1);
    std::cout << "Enter admin password: ";
    std::getline(std::cin, adminPassword);
    adminPassword.erase(adminPassword.find_last_not_of(" \n\r\t") + 1);
    std::cout << "Credentials set. Username: " << adminUsername << ", Password: " << adminPassword << std::endl;
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

bool TFTPServer::authenticateClient(SOCKET client_socket) {
    char buffer[1024];
    std::string username, password;

    // Request username
    std::string requestUsername = "Enter username: ";
    send(client_socket, requestUsername.c_str(), requestUsername.size(), 0);
    int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';  // Null-terminate the buffer
        username = std::string(buffer);
        // Trim trailing whitespace and newline characters
        username.erase(username.find_last_not_of(" \n\r\t") + 1);
    }

    // Request password
    std::string requestPassword = "Enter password: ";
    send(client_socket, requestPassword.c_str(), requestPassword.size(), 0);
    bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        password = std::string(buffer);
        // Trim trailing whitespace and newline characters
        password.erase(password.find_last_not_of(" \n\r\t") + 1);
    }

    // Debugging Output (Optional): Print stored and entered credentials
    std::cout << "Stored Username: [" << adminUsername << "], Entered Username: [" << username << "]\n";
    std::cout << "Stored Password: [" << adminPassword << "], Entered Password: [" << password << "]\n";

    // Validate credentials
    if (username == adminUsername && password == adminPassword) {
        std::string successMsg = "230 User logged in, proceed.\n";
        send(client_socket, successMsg.c_str(), successMsg.size(), 0);
        return true;
    }
    else {
        std::string errorMsg = "530 Incorrect login.\n";
        send(client_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return false;
    }
}

void TFTPServer::handleClient(SOCKET client_socket) {
    // Send the initial welcome message (220) to the client
    std::string welcomeMsg = "220 Welcome to TuneFTP!\r\n";
    int sentBytes = send(client_socket, welcomeMsg.c_str(), welcomeMsg.size(), 0);

    if (sentBytes == SOCKET_ERROR) {
        std::cerr << "Failed to send welcome message (220) to client.\n";
        closesocket(client_socket);
        return;
    }
    std::cout << "Sent welcome message to client: " << welcomeMsg;

    char buffer[1024];
    std::string username;
    bool isAuthenticated = false;

    // Handle USER and PASS commands for login
    while (!isAuthenticated) {
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_read == SOCKET_ERROR) {
            std::cerr << "Failed to receive data from client.\n";
            closesocket(client_socket);
            return;
        }
        else if (bytes_read == 0) {
            // Client closed the connection
            closesocket(client_socket);
            return;
        }

        buffer[bytes_read] = '\0';
        std::string command(buffer);
        std::cout << "Received command from client: " << command;

        if (command.find("USER") == 0) {
            // Extract the username and respond with 331 to prompt for password
            username = command.substr(5);  // Assume "USER username"
            std::string userResponse = "331 Username OK, need password.\r\n";
            send(client_socket, userResponse.c_str(), userResponse.size(), 0);
            std::cout << "Sent response to USER command: " << userResponse;
        }
        else if (command.find("PASS") == 0) {
            // Extract the password and validate
            std::string password = command.substr(5);  // Assume "PASS password"
            if (username == adminUsername && password == adminPassword) {
                std::string passResponse = "230 User logged in, proceed.\r\n";
                send(client_socket, passResponse.c_str(), passResponse.size(), 0);
                std::cout << "Sent login success response: " << passResponse;
                isAuthenticated = true;  // Authentication successful
            }
            else {
                std::string failResponse = "530 Login incorrect.\r\n";
                send(client_socket, failResponse.c_str(), failResponse.size(), 0);
                std::cerr << "Failed login attempt.\n";
                closesocket(client_socket);  // Close connection on failed login
                return;
            }
        }
        else {
            // If not yet authenticated, enforce login with USER and PASS
            std::string loginPrompt = "530 Please login with USER and PASS.\r\n";
            send(client_socket, loginPrompt.c_str(), loginPrompt.size(), 0);
            std::cout << "Sent login prompt to client: " << loginPrompt;
        }
    }

    // If authenticated, proceed to handle FTP commands
    handleCommands(client_socket);

    // Close the client socket after the session ends
    closesocket(client_socket);
    logConnection(client_socket, false);  // Log disconnection
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
            sendResponse(client_socket, "331 Username OK, need password.\r\n");  // Respond with 331 to prompt for password
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
        else if (!loggedIn) {
            // If not logged in, reject commands other than USER and PASS
            sendResponse(client_socket, "530 Please login with USER and PASS.\r\n");
        }
        else if (command.find("LIST") == 0) {
            handleLIST(client_socket);
        }
        else if (command.find("RETR") == 0) {
            handleRETR(client_socket, command.substr(5));
        }
        else if (command.find("STOR") == 0) {
            handleSTOR(client_socket, command.substr(5));
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