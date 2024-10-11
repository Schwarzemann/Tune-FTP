#ifndef TFTP_DOWNLOAD_HANDLER_H
#define TFTP_DOWNLOAD_HANDLER_H

#include <string>

class TFTPDownloadHandler {
public:
    static void handleDownload(int client_socket, const std::string& filePath);
};

#endif // TFTP_DOWNLOAD_HANDLER_H