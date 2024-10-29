# Tune FTP
Yet another FTP server written by a retard.

## How to build

I didn't create a solution for this project so you need to do the compilation manually through the developer console.
The command given below is pretty straightforward.

```
cl /EHsc main.cpp TFTPServer.cpp TFTPUploadHandler.cpp TFTPDownloadHandler.cpp /FeTuneFTP.exe /link ws2_32.lib
```
