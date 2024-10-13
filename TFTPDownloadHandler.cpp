#include "TFTPDownloadHandler.h"
#include <winsock2.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

void TFTPDownloadHandler::handleDownload(SOCKET client_socket, const std::string& filePath) {
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile) {
        std::string response = "Failed to open file for download\n";
        send(client_socket, response.c_str(), response.size(), 0);
        return;
    }

    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        std::string response = "Failed to retrieve file size\n";
        send(client_socket, response.c_str(), response.size(), 0);
        inputFile.close();
        return;
    }

    size_t totalFileSize = fileStat.st_size;
    size_t totalBytesSent = 0;
    char buffer[1024];

    while (inputFile.read(buffer, sizeof(buffer)) || inputFile.gcount() > 0) {
        int bytes_read = inputFile.gcount();
        int bytes_sent = send(client_socket, buffer, bytes_read, 0);
        if (bytes_sent == SOCKET_ERROR) {
            std::cerr << "Error writing to client.\n";
            inputFile.close();
            return;
        }
        totalBytesSent += bytes_sent;
    }

    inputFile.close();
    std::string response = "File downloaded successfully\n";
    send(client_socket, response.c_str(), response.size(), 0);
}
