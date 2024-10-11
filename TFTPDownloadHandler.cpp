#include "TFTPDownloadHandler.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>

void TFTPDownloadHandler::handleDownload(int client_socket, const std::string& filePath) {
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile) {
        std::string response = "Failed to open file for download\n";
        write(client_socket, response.c_str(), response.size());
        return;
    }

    char buffer[1024];
    while (inputFile.read(buffer, sizeof(buffer))) {
        write(client_socket, buffer, inputFile.gcount());
    }

    inputFile.close();
    std::string response = "File downloaded successfully\n";
    write(client_socket, response.c_str(), response.size());
}
