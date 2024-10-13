#include "TFTPUploadHandler.h"
#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <cstring>

#pragma comment(lib, "Ws2_32.lib")  // Link Winsock library

// Function to handle each client's upload in a separate thread
void handleClientUpload(SOCKET client_socket, const std::string filePath) {
    std::ofstream outputFile(filePath, std::ios::binary);
    if (!outputFile) {
        std::string response = "Failed to open file for upload\n";
        send(client_socket, response.c_str(), response.size(), 0);  // Send error message
        closesocket(client_socket);
        return;
    }

    // Notify client that upload is starting
    std::string startMsg = "Starting file upload\n";
    send(client_socket, startMsg.c_str(), startMsg.size(), 0);

    char buffer[1024];
    int totalBytesReceived = 0;
    int bytes_read;

    // File upload loop
    while (true) {
        bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);  // Read data from client
        if (bytes_read == SOCKET_ERROR) {
            std::cerr << "Error receiving data from client: " << WSAGetLastError() << "\n";
            break;
        }
        else if (bytes_read == 0) {
            // Client closed the connection
            break;
        }

        totalBytesReceived += bytes_read;

        // Write the received data to the file
        outputFile.write(buffer, bytes_read);

        // Check for write errors
        if (!outputFile.good()) {
            std::string errorMsg = "Failed to write to file\n";
            send(client_socket, errorMsg.c_str(), errorMsg.size(), 0);
            break;
        }

        // Progress tracking (optional)
        std::string progressMsg = "Received " + std::to_string(totalBytesReceived) + " bytes\n";
        send(client_socket, progressMsg.c_str(), progressMsg.size(), 0);
    }

    outputFile.close();

    if (totalBytesReceived > 0) {
        // File uploaded successfully
        std::string successMsg = "File uploaded successfully, " + std::to_string(totalBytesReceived) + " bytes received.\n";
        send(client_socket, successMsg.c_str(), successMsg.size(), 0);
        std::cout << "File upload completed: " << filePath << ", Total bytes: " << totalBytesReceived << "\n";
    }
    else {
        // Upload failed
        std::string errorMsg = "File upload failed\n";
        send(client_socket, errorMsg.c_str(), errorMsg.size(), 0);
        std::cerr << "File upload failed for: " << filePath << "\n";
    }

    // Close the socket connection
    closesocket(client_socket);
}

// Public function to start the upload in a separate thread
void TFTPUploadHandler::handleUpload(SOCKET client_socket, const std::string& filePath) {
    // Create a new thread to handle the client's upload
    std::thread uploadThread(handleClientUpload, client_socket, filePath);

    // Detach the thread to let it run independently
    uploadThread.detach();
}
