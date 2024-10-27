#include "TFTPDownloadHandler.h"
#include <winsock2.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <unordered_map>
#include <mutex>

#pragma comment(lib, "Ws2_32.lib")  // Link Winsock library

// Mutex to guard access to ongoing downloads and connected peers
std::mutex downloadMutex;
std::unordered_map<std::string, std::vector<SOCKET>> ongoingDownloads;  // Track sockets for each active download

void TFTPDownloadHandler::handleDownload(SOCKET data_socket, SOCKET client_socket, const std::string& filePath, const std::string& mode) {
    std::thread downloadThread([=]() {
        std::ifstream inputFile;

        // Open file based on mode
        if (mode == "binary") {
            inputFile.open(filePath, std::ios::binary);
        }
        else if (mode == "ascii") {
            inputFile.open(filePath);
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

        // Register the client in ongoing downloads
        {
            std::lock_guard<std::mutex> lock(downloadMutex);
            ongoingDownloads[filePath].push_back(client_socket);
        }

        // Start reading from file and sending over the data socket
        while (inputFile.read(buffer, sizeof(buffer)) || inputFile.gcount() > 0) {
            int bytesToSend = static_cast<int>(inputFile.gcount());
            int bytesSent = send(data_socket, buffer, bytesToSend, 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << "Error sending file data to client.\n";
                send(client_socket, "426 Connection closed; transfer aborted.\r\n", 44, 0);
                inputFile.close();

                // Remove client from ongoing downloads
                std::lock_guard<std::mutex> lock(downloadMutex);
                auto& clients = ongoingDownloads[filePath];
                clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
                if (clients.empty()) ongoingDownloads.erase(filePath);

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

        // Remove client from ongoing downloads
        {
            std::lock_guard<std::mutex> lock(downloadMutex);
            auto& clients = ongoingDownloads[filePath];
            clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
            if (clients.empty()) ongoingDownloads.erase(filePath);
        }

        // Close the data connection
        closesocket(data_socket);
        });

    downloadThread.detach();  // Detach to allow independent operation
}

void TFTPDownloadHandler::p2pDownload(SOCKET client_socket, const std::string& filePath) {
    std::lock_guard<std::mutex> lock(downloadMutex);

    auto it = ongoingDownloads.find(filePath);
    if (it != ongoingDownloads.end() && !it->second.empty()) {
        // Inform client of P2P availability
        std::string p2pMsg = "150 File is available for peer-to-peer transfer.\r\n";
        send(client_socket, p2pMsg.c_str(), p2pMsg.size(), 0);

        // For simplicity, assume the first client in the list will act as the peer
        SOCKET peerSocket = it->second.front();

        // Initiate transfer from peer
        char buffer[1024];
        int bytesReceived;
        while ((bytesReceived = recv(peerSocket, buffer, sizeof(buffer), 0)) > 0) {
            send(client_socket, buffer, bytesReceived, 0);
        }

        std::string completionMsg = "226 P2P Transfer complete.\r\n";
        send(client_socket, completionMsg.c_str(), completionMsg.size(), 0);
        std::cout << "P2P transfer to client completed for file: " << filePath << "\n";
    }
    else {
        std::string noPeerMsg = "425 No peer available for transfer.\r\n";
        send(client_socket, noPeerMsg.c_str(), noPeerMsg.size(), 0);
        std::cerr << "No peer available for P2P download of: " << filePath << "\n";
    }
}
