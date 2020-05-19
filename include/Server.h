/**
  文件注释样例，参考： http://www.edparrish.net/common/cppdoc.html
  socket服务端声明

  @project netTester
  @author Tao Zhang, Tao, Tao
  @since 2020/4/26
  @version 0.1.3 2020/4/30
*/
#pragma once
#include "UdpConnection.h"
#include "seeker/loggerApi.h"

class Server {

  UdpConnection conn{};

  uint16_t currentTest{0};

  void bandwidthTest(int testSeconds);

 public:
    SOCKET serverSocket;
    sockaddr_in clientAddr;
    int clientAddrSize;
  Server(int p);

  void start();

  void close();

};