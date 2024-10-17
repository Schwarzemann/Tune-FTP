#include "TFTPUploadHandler.h"
#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <cstring>

#pragma comment(lib, "Ws2_32.lib")  // Link Winsock library

void TFTPUploadHandler::handleUpload(SOCKET data_socket, SOCKET client_socket, const std::string& filePath, const std::string& mode) {
    std::ofstream outputFile;

    // Handle binary or ASCII mode
    if (mode == "binary") {
        outputFile.open(filePath, std::ios::binary);
    }
    else if (mode == "ascii") {
        outputFile.open(filePath);  // ASCII mode for text files
    }
    else {
        std::cerr << "Unsupported transfer mode.\n";
        send(client_socket, "550 Unsupported transfer mode.\r\n", 33, 0);
        return;
    }

    if (!outputFile) {
        std::cerr << "Failed to open file for upload: " << filePath << "\n";
        send(client_socket, "550 Could not open file for upload.\r\n", 38, 0);
        return;
    }

    // Notify client that the upload is starting
    std::string transferMsg = "150 Opening " + mode + " mode data connection for upload.\r\n";
    send(client_socket, transferMsg.c_str(), transferMsg.size(), 0);

    char buffer[1024];
    int totalBytesReceived = 0;
    int bytesReceived;

    // Receive data from the client and write it to the file
    while (true) {
        bytesReceived = recv(data_socket, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error receiving data from client.\n";
            send(client_socket, "426 Connection closed; transfer aborted.\r\n", 44, 0);
            break;
        }
        else if (bytesReceived == 0) {
            // Client has closed the connection
            break;
        }

        totalBytesReceived += bytesReceived;
        outputFile.write(buffer, bytesReceived);

        if (!outputFile.good()) {
            std::cerr << "Error writing to file during upload: " << filePath << "\n";
            send(client_socket, "550 Failed to write to file.\r\n", 31, 0);
            break;
        }

        // Log upload progress
        std::cout << "Received " << totalBytesReceived << " bytes so far.\n";
    }

    outputFile.close();

    if (totalBytesReceived > 0) {
        std::cout << "File upload completed: " << filePath << " (" << totalBytesReceived << " bytes)\n";
        send(client_socket, "226 Transfer complete.\r\n", 24, 0);
    }
    else {
        std::cerr << "File upload failed: " << filePath << "\n";
        send(client_socket, "550 Upload failed.\r\n", 20, 0);
    }

    // Close the data connection
    closesocket(data_socket);
}