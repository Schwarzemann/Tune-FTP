#ifndef TFTP_UPLOAD_HANDLER_H
#define TFTP_UPLOAD_HANDLER_H

#include <string>
#include <winsock2.h>

class TFTPUploadHandler {
public:
    static void handleUpload(SOCKET data_socket, SOCKET client_socket, const std::string& filePath, const std::string& mode);
};

#endif // TFTP_UPLOAD_HANDLER_H