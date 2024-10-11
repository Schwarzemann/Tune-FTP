#ifndef TFTP_SERVER_H
#define TFTP_SERVER_H

#include <string>
#include <map>

class TFTPServer {
public:
    TFTPServer(int port);
    ~TFTPServer();

    void start();
    void stop();

private:
    int tuneFTP_port;
    int tuneFTP_socket;
    std::map<int, std::string> tuneFTP_sessions;

    void handleConnection(int client_socket);
    void processCommand(int client_socket, const std::string& command);
    void switchDirectory(int client_socket, const std::string& path);
};

#endif // TFTP_SERVER_H
