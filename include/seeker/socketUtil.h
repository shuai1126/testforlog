#pragma once


#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <arpa/inet.h>
#include <sys/socket.h>
#define SOCKET int
#define SOCKET_ERROR (-1)

namespace seeker {

class SocketUtil {
 public:
  static sockaddr_in createAddr(int port, const std::string& host = "") {
    sockaddr_in addr = {0};
    const char * hostData = host.empty() ? nullptr : host.c_str();
    addr.sin_family = AF_INET;
    setSocketAddr(&addr, hostData, port);
    return addr;
  }

  static void setSocketAddr(sockaddr_in* addr, const char* ip, const int port = -1) {
    if (ip == nullptr) {
      (*addr).sin_addr.s_addr = htonl(INADDR_ANY);

    } else {
      (*addr).sin_addr.s_addr = inet_addr(ip);
    }

    if(port != -1) {
      (*addr).sin_port = htons(port);
    }

  }

};



}  // namespace seeker
