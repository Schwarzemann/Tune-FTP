#include "TFTPUploadHandler.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>

void TFTPUploadHandler::handleUpload(int client_socket, const std::string& filePath) {
    std::ofstream outputFile(filePath, std::ios::binary);
    if (!outputFile) {
        std::string response = "Failed to open file for upload\n";
        write(client_socket, response.c_str(), response.size());
        return;
    }

    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(client_socket, buffer, sizeof(buffer))) > 0) {
        outputFile.write(buffer, bytes_read);
    }

    outputFile.close();
    std::string response = "File uploaded successfully\n";
    write(client_socket, response.c_str(), response.size());
}