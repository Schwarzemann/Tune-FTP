#include "TFTPServer.h"
#include <iostream>

int main() {
    int port = 21; // FTP default port
    TFTPServer tuneFTP(port);

    std::cout << "Starting TuneFTP Server..." << std::endl;
    tuneFTP.start();

    return 0;
}