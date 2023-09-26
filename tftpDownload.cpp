#include "exp3.h"
extern ofstream logOut;
extern string errorTable[8];

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
    uint64_t transferredSize=0;
    clock_t startTime;
    clock_t currentTime;
public:
    tftpDownload(string name, string ip, int p, string m);
    void recvFile();
    ~tftpDownload();
};

tftpDownload::tftpDownload(string name, string ip, int p, string m)
{
    fileName = name;
    ipAddress = ip;
    port = p;
    mode = m;
}

void tftpDownload::recvFile()
{
    
}

tftpDownload::~tftpDownload()
{
}
