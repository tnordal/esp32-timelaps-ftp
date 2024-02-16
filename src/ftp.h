#include "ESP32_FTPClient.h"

ESP32_FTPClient ftp(ftp_server, ftp_port, ftp_user, ftp_pass);

void uploadFile(unsigned char *data, int length, const char *name) {
    DBG("Uploading via FTP");

    ftp.OpenConnection();
    // DBG("Connected");

    // DBG("Change to Binary");
    ftp.InitFile("Type I");

    // DBG("Change Working Directory to " + String(ftp_remote_dir));
    ftp.ChangeWorkDir(ftp_remote_dir);
    
    // DBG("New File " + String(name));
    ftp.NewFile(name);
    
    // DBG("Uploading " + String(name));
    ftp.WriteData(data, length);
    
    ftp.CloseFile();
    DBG("");
}
