#include "TFTPDownloadHandler.h"
#include <winsock2.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")  // Link Winsock library

void TFTPDownloadHandler::handleDownload(SOCKET data_socket, SOCKET client_socket, const std::string& filePath, const std::string& mode) {
    std::ifstream inputFile;

    // Open the file based on the mode (binary or ASCII)
    if (mode == "binary") {
        inputFile.open(filePath, std::ios::binary);
    }
    else if (mode == "ascii") {
        inputFile.open(filePath);  // Open as ASCII for text-based transfer
    }
    else {
        std::cerr << "Unsupported transfer mode.\n";
        send(client_socket, "550 Unsupported transfer mode.\r\n", 33, 0);
        return;
    }

    if (!inputFile) {
        std::cerr << "Failed to open file: " << filePath << "\n";
        send(client_socket, "550 File not found.\r\n", 22, 0);
        return;
    }

    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        std::cerr << "Failed to retrieve file size for: " << filePath << "\n";
        send(client_socket, "550 Could not retrieve file size.\r\n", 36, 0);
        inputFile.close();
        return;
    }

    size_t totalFileSize = fileStat.st_size;
    size_t totalBytesSent = 0;
    char buffer[1024];

    // Notify the client that the transfer is starting
    std::string transferMsg = "150 Opening " + mode + " mode data connection for " + filePath + "\r\n";
    send(client_socket, transferMsg.c_str(), transferMsg.size(), 0);

    // Start reading from file and sending over the data socket
    while (inputFile.read(buffer, sizeof(buffer)) || inputFile.gcount() > 0) {
        int bytesToSend = static_cast<int>(inputFile.gcount());
        int bytesSent = send(data_socket, buffer, bytesToSend, 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Error sending file data to client.\n";
            send(client_socket, "426 Connection closed; transfer aborted.\r\n", 44, 0);
            inputFile.close();
            return;
        }
        totalBytesSent += bytesSent;

        // Log progress
        std::cout << "Sent " << totalBytesSent << " bytes of " << totalFileSize << " bytes\n";
    }

    inputFile.close();
    std::cout << "File transfer completed: " << filePath << "\n";
    std::string completionMsg = "226 Transfer complete.\r\n";
    send(client_socket, completionMsg.c_str(), completionMsg.size(), 0);

    // Close the data connection
    closesocket(data_socket);
}