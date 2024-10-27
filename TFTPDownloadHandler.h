#ifndef TFTP_DOWNLOAD_HANDLER_H
#define TFTP_DOWNLOAD_HANDLER_H

#include <string>
#include <winsock2.h>

class TFTPDownloadHandler {
public:
    static void handleDownload(SOCKET data_socket, SOCKET client_socket, const std::string& filePath, const std::string& mode);
    static void p2pDownload(SOCKET client_socket, const std::string& filePath);
};

#endif // TFTP_DOWNLOAD_HANDLER_H