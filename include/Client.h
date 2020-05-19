/**
  文件注释样例，参考： http://www.edparrish.net/common/cppdoc.html
  socket客户端定义

  @project netTester
  @author Tao Zhang
  @since 2020/4/26
  @version 0.1.3.1 2020/5/2
*/

#pragma once
#include <string>
#include <atomic>
#include "UdpConnection.h"
//#include "seeker/socketUtil.h"
#include "Message.h"


using std::string;



class Client {
  UdpConnection conn;
  std::atomic<int> nextMid{0};

  void sendMsg(const Message& msg);

 public:
    SOCKET clientSocket;
    const sockaddr *serverAddr;
    int serverAddrSize;
  Client(const string& serverHost, int serverPort);

  void close();

  void startRtt(int testTimes, int packetSize);

  void startBandwidth(uint32_t bandwidth,
                      char bandwidthUnit,
                      int packetSize,
                      int testSeconds,
                      int reportInterval);
};
