#include "exp3.h"
extern ofstream logOut;
extern string errorTable[8];

tftpDownload::tftpDownload(string name, string ip, int p, string m)
{
    fileName = name;
    ipAddress = ip;
    port = p;
    mode = m;
}

void tftpDownload::recvFile()
{
    // 1. 创建socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        logOut<<time_now()<<"[ERROR] Create socket error." << endl;
        return;
    }
    // 2. 绑定本地地址
    struct sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(0);
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sock, (struct sockaddr*)&localAddr, sizeof(localAddr));
    // 3. 构造服务器地址
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());
    // 4. 构造请求报文
    // 4.1. 读取文件
    filePath=DOWNLOAD_PATH+fileName;
    ofstream fileOut(filePath, ios::out | ios::binary);
    if(!fileOut.is_open())
    {
        logOut<<time_now() <<"[ERROR] Cannot open file." << endl;
        return;
    }
    // 4.2. 构造请求报文
    uint16_t opcode = 1;
    opcode=htons(opcode);
    memcpy(sendBuf, &opcode, 2);
    // 4.3. 构造请求报文的文件名
    int i=0;
    for(; i<fileName.size(); i++)
    {
        sendBuf[2+i] = fileName[i];
    }
    // 4.4. 构造请求报文的模式
    i++;
    for(int j=0; j<mode.size(); j++)
    {
        sendBuf[2+i+j] = mode[j];
    }
    sendBuf[4+i+mode.size()] = 0;
    // 5. 发送请求报文
    int sendLen = sendto(sock, sendBuf, 4+i+mode.size()+1, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    // 6. 接收数据
    startTime = clock();
    socklen_t serverAddrLen = sizeof(serverAddr);
    while(true)
    {
        // 6.1. 接收数据
        int recvLen = recvfrom(sock, recvBuf, MAX_BUF, 0, (struct sockaddr*)&serverAddr, &serverAddrLen);
        if(recvLen < 0)
        {
            //超时
            if(resendTimes<MAX_RESEND_TIMES)
            {
                resendTimes++;
                logOut<<time_now()<<"[INFO] Resend ACK pack #"<<resendTimes<<"."<<endl;
                sendLen = sendto(sock, sendBuf, 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
                continue;
            }
            else
            {
                logOut<<time_now()<<"[ERROR] Receive data time out." << endl;
                return;
            }
            logOut<<time_now()<<"[ERROR] Unknown Receive data error." << endl;
            return;
        }
        // 6.2. 解析数据
        memcpy(&recvBlockNum, recvBuf+2, 2);
        recvBlockNum = ntohs(recvBlockNum);
        // 6.3. 判断是否为错误报文
        uint16_t tmpOpcode;
        memcpy(&tmpOpcode, recvBuf, 2);
        tmpOpcode = ntohs(tmpOpcode);
        if(tmpOpcode == 5)
        {
            uint16_t errorCode;
            memcpy(&errorCode, recvBuf+2, 2);
            errorCode = ntohs(errorCode);
            logOut<<time_now()<<"[ERROR] TFTP's error: "<<errorTable[errorCode]<<endl;
            return;
        }
        // 6.4. 判断是否为数据报文
        if(recvBlockNum == blockNum+1)
        {
            // 6.4.1. 写入文件
            fileOut.write(recvBuf+4, recvLen-4);
            // 6.4.2. 更新状态参数
            blockNum = recvBlockNum;
            transferredSize += recvLen-4;
            // 6.4.3. 发送ACK报文
            opcode = 4;
            opcode = htons(opcode);
            uint16_t nBlockNum = htons(blockNum);
            memcpy(sendBuf, &opcode, 2);
            memcpy(sendBuf+2, &nBlockNum, 2);
            sendLen = sendto(sock, sendBuf, 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
            logOut<<time_now()<<"[INFO] Received Data #"<<recvBlockNum<<", transferred size="<<transferredSize<<" Bytes."<<endl;
            // 6.4.4. 判断是否传输完成
            if(recvLen-4 < 512)
            {
                logOut<<time_now()<<"[INFO] File transfer completed."<<endl;
                break;
            }
        }
        else if(recvBlockNum == blockNum)
        {
            // 6.5.1. 重发ACK报文
            sendLen = sendto(sock, sendBuf, 4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        }
        else
        {
            logOut<<time_now()<<"[ERROR] Receive data error." << endl;
            return;
        }
    }
    // 7. 关闭文件
    fileOut.close();
    // 8. 关闭socket
    close(sock);
    // 9. 计算传输速度
    currentTime = clock();
    double time = (double)(currentTime - startTime) / CLOCKS_PER_SEC;
    double speed = (double)transferredSize / time;
    logOut<<time_now()<<"[INFO] File size: "<<transferredSize<<" bytes."<<endl;
    logOut<<time_now()<<"[INFO] Time: "<<time<<" s."<<endl;
    string unit="Bytes/s";
    if(speed>=1024&&speed<1024*1024)
    {
        speed/=1024;
        unit="KB/s";
    }
    else if(speed>=1024*1024&&speed<1024*1024*1024)
    {
        speed/=1024*1024;
        unit="MB/s";
    }
    else if(speed>=1024*1024*1024)
    {
        speed/=1024*1024*1024;
        unit="GB/s";
    }
    logOut<<time_now()<<"[INFO] Speed: "<<setiosflags(ios::fixed)<<setprecision(2)<<speed<<" "<<unit<<endl;
    return;
}

tftpDownload::~tftpDownload()
{
}
