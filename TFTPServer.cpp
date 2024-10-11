#include "TFTPServer.h"
#include "TFTPUploadHandler.h"
#include "TFTPDownloadHandler.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

TFTPServer::TFTPServer(int port) : tuneFTP_port(port), tuneFTP_socket(-1) {}

TFTPServer::~TFTPServer() {
    stop();
}

void TFTPServer::start() {
    tuneFTP_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tuneFTP_socket < 0) {
        std::cerr << "Error: Failed to create socket.\n";
        return;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(tuneFTP_port);

    if (bind(tuneFTP_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error: Bind failed.\n";
        return;
    }

    listen(tuneFTP_socket, 5);
    std::cout << "TuneFTP Server started on port " << tuneFTP_port << "\n";

    while (true) {
        int client_socket = accept(tuneFTP_socket, nullptr, nullptr);
        if (client_socket >= 0) {
            handleConnection(client_socket);
        }
    }
}

void TFTPServer::stop() {
    if (tuneFTP_socket >= 0) {
        close(tuneFTP_socket);
    }
}

void TFTPServer::handleConnection(int client_socket) {
    char buffer[1024];
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        std::string command(buffer);
        processCommand(client_socket, command);
    }
    close(client_socket);
}

void TFTPServer::processCommand(int client_socket, const std::string& command) {
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
        write(client_socket, response.c_str(), response.size());
    }
}

void TFTPServer::switchDirectory(int client_socket, const std::string& path) {
    if (chdir(path.c_str()) == 0) {
        std::string response = "Directory changed to " + path + "\n";
        write(client_socket, response.c_str(), response.size());
    }
    else {
        std::string response = "Failed to change directory\n";
        write(client_socket, response.c_str(), response.size());
    }
}