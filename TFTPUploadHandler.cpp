#include "TFTPUploadHandler.h"
#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <cstring>

#pragma comment(lib, "Ws2_32.lib")  // Link Winsock library

void TFTPUploadHandler::handleUpload(SOCKET client_socket, const std::string& filePath) {
    std::ofstream outputFile(filePath, std::ios::binary);
    if (!outputFile) {
        std::string response = "Failed to open file for upload\n";
        send(client_socket, response.c_str(), response.size(), 0);  // Use send instead of write
        return;
    }

    char buffer[1024];
    int bytes_read;
    while ((bytes_read = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {  // Use recv instead of read
        outputFile.write(buffer, bytes_read);
    }

    outputFile.close();
    std::string response = "File uploaded successfully\n";
    send(client_socket, response.c_str(), response.size(), 0);  // Use send instead of write
}