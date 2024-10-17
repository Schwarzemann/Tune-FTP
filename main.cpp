#include "TFTPServer.h"
#include <iostream>
#include <cstdlib>  // For exit() and atoi()
#include <csignal>  // For signal handling (optional)
#include <cstring>  // For strerror

TFTPServer* tuneFTP = nullptr;  // Pointer to the FTP server instance

// Signal handler for clean server shutdown (optional)
void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down the FTP server gracefully..." << std::endl;
    if (tuneFTP) {
        tuneFTP->stop();
    }
    exit(signum);
}

int main(int argc, char* argv[]) {
    int port = 21;  // Default FTP port

    // Check if a custom port is provided as a command-line argument
    if (argc > 1) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Invalid port number provided. Using default port 21." << std::endl;
            port = 21;
        }
    }

    // Initialize the FTP server
    tuneFTP = new TFTPServer(port);

    // Register signal handler to cleanly stop the server (optional)
    signal(SIGINT, signalHandler);  // Handle Ctrl+C interrupt for graceful shutdown

    // Manually trigger server configuration phase before starting
    tuneFTP->configureServer();

    std::cout << "Starting TuneFTP Server on port " << port << "..." << std::endl;

    // Start the server and check for errors
    try {
        tuneFTP->start();
    }
    catch (const std::exception& ex) {
        std::cerr << "Error occurred while starting the FTP server: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}