#include "exp3.h"
extern ofstream logOut;
extern string errorTable[8];

tftpUpload::tftpUpload(string path, string ip, int p, string m)
{
    filePath = path;
    ipAddress = ip;
    port = p;
    mode = m;
}

void tftpUpload::sendFile()
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
    ifstream fileIn(filePath, ios::in | ios::binary);
    if(!fileIn.is_open())
    {
        logOut<<time_now() <<"[ERROR] Cannot open file." << endl;
        return;
    }
    fileIn.seekg(0, ios::end);
    fileSize = fileIn.tellg();
    fileIn.seekg(0, ios::beg);
    // 4.2. 构造请求报文
    uint16_t opcode = 2;
    opcode=htons(opcode);
    memcpy(sendBuf, &opcode, 2);
    // 4.3. 构造请求报文的文件名
    fileName=filePath.substr(filePath.find_last_of('/')+1);
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
    if(sendLen < 0)
    {
        logOut<<time_now() << "[ERROR] Cannot send WRQ pack." << endl;
        return;
    }
    // 6. 接收ACK报文
    socklen_t serverAddrLen = sizeof(serverAddr);
    while(1)
    {
        // 6.1. 设置超时
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        // 6.2. 接收ACK报文
        int recvLen = recvfrom(sock, recvBuf, MAX_BUF, 0, (struct sockaddr*)&serverAddr, &serverAddrLen);
        if(recvLen < 0){
            if(errno == EAGAIN){
                // 超时重传
                resendTimes++;
                if(resendTimes > 3){
                    logOut<<time_now() << "[ERROR] Time out" << endl;
                    return;
                }
                logOut<<time_now() << "[INFO] resend " << resendTimes << " times." << endl;
                sendLen = sendto(sock, sendBuf, 4+i+mode.size()+1, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
                if(sendLen < 0){
                    logOut<<time_now() << "[ERROR] Cannot send WRQ pack. Resending "<<resendTimes<<"times." << endl;
                    return;
                }
                continue;
            }
            logOut<<time_now() << "[ERROR] Unknown recv error." << endl;
            return;
        }
        // 6.3. 检查ACK报文
        if(recvBuf[0] == 0 && recvBuf[1] == 4){
            recvBlockNum = (recvBuf[2] << 8) + recvBuf[3];
            if(recvBlockNum == blockNum){
                logOut<<time_now() << "[INFO] Received ACK #" << recvBlockNum << endl;
                break;
            }
        }
        else if(recvBuf[0] == 0 && recvBuf[1] == 5){
            logOut<<time_now() << "[ERROR] Received ERROR #" << (recvBuf[2] << 8) + recvBuf[3] <<" "<<errorTable[int((recvBuf[2] << 8) + recvBuf[3])] << endl;
            return;
        }
    }
    // 7. 发送文件
    startTime = clock();
    blockNum++;
    while(1)
    {
        // 7.1. 读取文件
        fileIn.read(sendBuf+4, MAX_BUF-4);
        int readLen = fileIn.gcount();
        // 7.2. 构造数据报文
        opcode=3;
        opcode=htons(opcode);
        memcpy(sendBuf, &opcode, 2);
        uint16_t nBlockNum=htons(blockNum);
        memcpy(sendBuf+2, &nBlockNum, 2);
        // 7.3. 发送数据报文
        sendLen = sendto(sock, sendBuf, readLen+4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if(sendLen < 0){
            logOut<<time_now() << "[ERROR] Cannot send Data #"<<blockNum<<"." << endl;
            return;
        }
        // 7.4. 接收ACK报文
        while(1){
            // 7.4.1. 设置超时
            struct timeval timeout;
            timeout.tv_sec = TIMEOUT;
            timeout.tv_usec = 0;
            setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
            // 7.4.2. 接收ACK报文
            int recvLen = recvfrom(sock, recvBuf, MAX_BUF, 0, (struct sockaddr*)&serverAddr, &serverAddrLen);
            if(recvLen < 0)
            {
                if(errno == EAGAIN)
                {
                    // 超时重传
                    resendTimes++;
                    if(resendTimes > 3)
                    {
                        logOut<<time_now() << "[ERROR] Time out" << endl;
                        return;
                    }
                    logOut<<time_now() << "[INFO] resend " << resendTimes << " times." << endl;
                    sendLen = sendto(sock, sendBuf, readLen+4, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
                    if(sendLen < 0)
                    {
                        logOut<<time_now() << "[ERROR] Cannot send WRQ pack. Resending "<<resendTimes<<"times." << endl;
                        return;
                    }
                    continue;
                }
                logOut<<time_now() << "[ERROR] Unknown recv error." << endl;
                return;
            }
            // 7.4.3. 检查ACK报文
            if(recvBuf[0] == 0 && recvBuf[1] == 4)
            {
                recvBlockNum = (recvBuf[2] << 8) + recvBuf[3];
                if(recvBlockNum == blockNum)
                {
                    logOut << time_now() << "[INFO] Received ACK #" << recvBlockNum << endl;
                    break;
                }
            }
            else if(recvBuf[0] == 0 && recvBuf[1] == 5)
            {
                logOut<<time_now() << "[ERROR] Received ERROR #" << (recvBuf[2] << 8) + recvBuf[3] <<" "<<errorTable[int((recvBuf[2] << 8) + recvBuf[3])] << endl;
                return;
            }
        }
        transferredSize += readLen;
        currentTime = clock();
        double speed=(transferredSize*CLOCKS_PER_SEC)/(currentTime-startTime);// Bps
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
        logOut << time_now() << "[INFO] Transferred " << transferredSize<<"/"<<fileSize << " Bytes. Speed "<<setiosflags(ios::fixed)<<setprecision(2)<<speed<<" "<<unit<< endl;
        // 7.5. 检查是否传输完成
        if(readLen < MAX_BUF-4)
        {
            logOut << time_now() << "[INFO] File transfer complete." << endl;
            break;
        }
        // 7.6. 更新状态参数
        blockNum++;
    }
    // 8. 关闭socket
    close(sock);
}

tftpUpload::~tftpUpload()
{

}
