#ifndef TFTP_UPLOAD_HANDLER_H
#define TFTP_UPLOAD_HANDLER_H

#include <string>
#include <winsock2.h>

class TFTPUploadHandler {
public:
    static void handleUpload(SOCKET client_socket, const std::string& filePath);  // Use SOCKET instead of int
};

#endif // TFTP_UPLOAD_HANDLER_H