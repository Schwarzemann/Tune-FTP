#ifndef TFTP_UPLOAD_HANDLER_H
#define TFTP_UPLOAD_HANDLER_H

#include <string>

class TFTPUploadHandler {
public:
    static void handleUpload(int client_socket, const std::string& filePath);
};

#endif // TFTP_UPLOAD_HANDLER_H