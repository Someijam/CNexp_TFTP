#include "exp3.h"

ofstream logOut;
string errorTable[8] = {
    "Not defined, see error message (if any).",
    "File not found.",
    "Access violation.",
    "Disk full or allocation exceeded.",
    "Illegal TFTP operation.",
    "Unknown transfer ID.",
    "File already exists.",
    "No such user."};

string time_now()
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    string time_now = to_string(1900 + ltm->tm_year) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday) + " " + to_string(ltm->tm_hour) + ":" + to_string(ltm->tm_min) + ":" + to_string(ltm->tm_sec);
    return time_now;
}

int main(int argc, char const *argv[])
{
    // Usage
    // ./main put filePath ipAddress port mode
    // ./main get fileName ipAddress port mode
    if (argc != 6)
    {
        cout << "Usage: ./main put filePath ipAddress port mode" << endl;
        cout << "Usage: ./main get fileName ipAddress port mode" << endl;
        return 1;
    }
    logOut.open(time_now() + ".log", ios::out);
    if (!logOut.is_open())
    {
        cout << "log file open error" << endl;
        return 1;
    }
    string mode = argv[5];
    if (mode != "octet" && mode != "netascii")
    {
        cout << "mode error" << endl;
        return 1;
    }
    if (argv[1] == string("put"))
    {
        tftpUpload upload(argv[2], argv[3], atoi(argv[4]), argv[5]);
        upload.sendFile();
    }
    else if (argv[1] == string("get"))
    {
        // tftpDownload download(argv[2], argv[3], atoi(argv[4]), argv[5]);
        // download.recvFile();
    }
    else
    {
        cout << "Usage: ./main put filePath ipAddress port mode" << endl;
        cout << "Usage: ./main get fileName ipAddress port mode" << endl;
        return 1;
    }
    return 0;
}
