#include<iostream>
#include<fstream>
#include<iomanip>
#include<ctime>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>

using namespace std;

#define TIMEOUT 3
#define MAX_RESEND_TIMES 5
#define MAX_BUF 516
#define DOWNLOAD_PATH "/Users/someijamling/Downloads/"

extern ofstream logOut;
extern string errorTable[8];

string time_now();

class tftpUpload
{
private:
    // 用户参数
    string filePath;
    string ipAddress;
    int port;
    string mode;
    // 状态参数
    string fileName;
    char sendBuf[MAX_BUF]={0};
    char recvBuf[MAX_BUF]={0};
    uint16_t blockNum=0;
    uint16_t recvBlockNum=0;
    uint16_t resendTimes=0;
    bool resendFlag=false;
    //其他参数
    clock_t startTime;
    clock_t currentTime;
    string processBar="[                                                  ]";
public:
    uint64_t fileSize=0;
    uint64_t transferredSize=0;
    tftpUpload(string path, string ip, int p, string m);
    void sendFile();
    ~tftpUpload();
};

class tftpDownload
{
private:
    // 用户参数
    string fileName;
    string ipAddress;
    int port;
    string mode;
    // 状态参数
    uint64_t fileSize=0;
    char sendBuf[MAX_BUF]={0};
    char recvBuf[MAX_BUF]={0};
    uint16_t blockNum=0;
    uint16_t recvBlockNum=0;
    uint16_t resendTimes=0;
    bool resendFlag=false;
    //其他参数
    string filePath;
    uint64_t transferredSize=0;
    clock_t startTime;
    clock_t currentTime;
public:
    tftpDownload(string name, string ip, int p, string m);
    void recvFile();
    ~tftpDownload();
};