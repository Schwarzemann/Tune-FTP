#ifndef TFTP_SERVER_H
#define TFTP_SERVER_H

#include <string>
#include <winsock2.h>
#include <vector>

class TFTPServer {
public:
    TFTPServer(int port);
    ~TFTPServer();

    void configureServer();
    void start();
    void stop();

private:
    int tuneFTP_port;
    SOCKET tuneFTP_socket;   // Control channel
    SOCKET data_socket;      // Data channel for transfers (active or passive)

    std::string adminUsername;
    std::string adminPassword;
    std::string accessDirectory;

    void handleClient(SOCKET client_socket);
    void handleCommands(SOCKET client_socket);
    void handlePORT(SOCKET client_socket, const std::string& params);
    void handlePASV(SOCKET client_socket);
    void handleLIST(SOCKET client_socket);
    void handleRETR(SOCKET client_socket, const std::string& filename);
    void handleSTOR(SOCKET client_socket, const std::string& filename);
    void handleQUIT(SOCKET client_socket);

    void sendResponse(SOCKET client_socket, const std::string& response);
    void openDataChannel(SOCKET client_socket);
    void logConnection(SOCKET client_socket, bool isConnected);
    void closeDataChannel();
    SOCKET acceptDataConnection(SOCKET client_socket);
};

#endif