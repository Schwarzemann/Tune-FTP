#ifndef TFTP_SERVER_H
#define TFTP_SERVER_H

#include <string>
#include <winsock2.h>

class TFTPServer {
public:
    TFTPServer(int port);
    ~TFTPServer();

    void start();
    void stop();

private:
    int tuneFTP_port;
    SOCKET tuneFTP_socket;

    void handleConnection(SOCKET client_socket);
    void processCommand(SOCKET client_socket, const std::string& command);
    void switchDirectory(SOCKET client_socket, const std::string& path);

    void logConnection(SOCKET client_socket, bool isConnected);
};

#endif // TFTP_SERVER_H
