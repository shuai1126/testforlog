
#pragma once
//#include <config.h>
#include <iostream>
#include <atomic>
#include <cstring>
#include "common.h"

#define MSG_SEND_BUF_SIZE 2048

enum class MessageType {
  testRequest = 1,
  testConfirm = 2,
  rttTestMsg = 3,
  bandwidthTestMsg = 4,
  bandwidthFinish = 5,
  bandwidthReport = 6,
};

class Message {
 protected:
  void writeHead(uint8_t* buf, size_t size) const {
    if (size < 15) {
      throw std::runtime_error("buf for head too small.");
    }
//    using seeker::ByteArray;
    ByteArray::writeData(buf + 0, msgType);
    ByteArray::writeData(buf + 1, testId);
    ByteArray::writeData(buf + 3, msgId);
    ByteArray::writeData(buf + 7, timestamp);
  }

  constexpr uint16_t headLen() const { return 15; }


 public:
  uint8_t msgType;    // index: 0
  uint16_t testId;    // index: 1
  int msgId;          // index: 3
  int64_t timestamp;  // index: 7

  Message() = default;

  Message(uint8_t* data) { reset(data); }

  Message(uint8_t mt, uint16_t tid, int mid)
      : msgType(mt), testId(tid), msgId(mid), timestamp(Time::microTime()) {}

  void reset(uint8_t* data) {
    ByteArray::readData(data + 0, msgType);
    ByteArray::readData(data + 1, testId);
    ByteArray::readData(data + 3, msgId);
    ByteArray::readData(data + 7, timestamp);
  }


  virtual size_t getLength() const = 0;

  virtual void getBinary(uint8_t* buf, size_t size) const = 0;

  static void getMsgType(uint8_t* data, uint8_t& v) {
    ByteArray::readData(data + 0, v);
  }

  static void getTestId(uint8_t* data, uint16_t& v) {
    ByteArray::readData(data + 1, v);
  }

  static void getMsgId(uint8_t* data, int& v) { ByteArray::readData(data + 3, v); }

  static void getTimestamp(uint8_t* data, int64_t& v) {
    ByteArray::readData(data + 7, v);
  }

  static void sendMsg(const Message& msg, SOCKET clientSocket, sockaddr_in serverAddr, int serverAddrSize) {
    static uint8_t buf[MSG_SEND_BUF_SIZE]{0};
    size_t len = msg.getLength();
    msg.getBinary(buf, MSG_SEND_BUF_SIZE);
    sendto(clientSocket, (char* )buf, len, 0, (sockaddr*)&serverAddr, serverAddrSize);
    memset(buf, 0, len);
  }

  static void replyMsg(const Message& msg, SOCKET serverSocket, sockaddr_in clientAddr, int clientAddrSize) {
    static uint8_t buf[MSG_SEND_BUF_SIZE]{0};
    size_t len = msg.getLength();
    msg.getBinary(buf, MSG_SEND_BUF_SIZE);
    sendto(serverSocket, (char* )buf, len, 0, (sockaddr*)&clientAddr, clientAddrSize);
    memset(buf, 0, len);
  }

  static int genMid() {
    static std::atomic<int> nextMid{0};
    return nextMid.fetch_add(1);
  }
};


enum class TestType {
  unknown,
  rtt,
  bandwidth,
};

class TestRequest : public Message {
 public:
  int testType;  //  1 for RTT, 2 for bandwidth.  index: 15.
  int testTime;  //  secondes.                    index: 19.

  static const uint8_t msgType = (uint8_t)MessageType::testRequest;

  TestRequest() = default;

  TestRequest(uint8_t* data) : Message(data) {
    ByteArray::readData(data + 15, testType);
    ByteArray::readData(data + 19, testTime);
  }

  TestRequest(TestType tType, int tTime, int mid)
      : Message(msgType, 0, mid), testType((int)tType), testTime(tTime) {}

  size_t getLength() const override { return headLen() + 8; }

  void getBinary(uint8_t* buf, size_t size) const override {
    if (size < getLength()) {
      throw std::runtime_error("buf too small.");
    }

    writeHead(buf, size);
    ByteArray::writeData(buf + 15, testType);
    ByteArray::writeData(buf + 19, testTime);
  }
};


class TestConfirm : public Message {
 public:
  int result;   //  1 for OK, 2 for NO           index: 15.
  int reMsgId;  //  reply mid.                   index: 19.

  static const uint8_t msgType = (uint8_t)MessageType::testConfirm;

  TestConfirm() = default;

  TestConfirm(uint8_t* data) : Message(data) {
    ByteArray::readData(data + 15, result);
    ByteArray::readData(data + 19, reMsgId);
  }

  TestConfirm(int rst, int reId, int mid)
      : Message(msgType, 0, mid), result(rst), reMsgId(reId) {}

  size_t getLength() const override { return headLen() + 8; }

  void getBinary(uint8_t* buf, size_t size) const override {
    if (size < getLength()) {
      throw std::runtime_error("buf too small.");
    }

    writeHead(buf, size);
    ByteArray::writeData(buf + 15, result);
    ByteArray::writeData(buf + 19, reMsgId);
  }
};


class RttTestMsg : public Message {
 public:
  int payloadLen;  //                       index: 15.

  static const uint8_t msgType = (uint8_t)MessageType::rttTestMsg;


  RttTestMsg(uint8_t* data) : Message(data) {
    ByteArray::readData(data + 15, payloadLen);
  }

  RttTestMsg(int packetSize, int testId, int mid)
      : Message(msgType, testId, mid), payloadLen(packetSize - 15) {}

  size_t getLength() const override { return headLen() + (size_t)payloadLen; }

  void getBinary(uint8_t* buf, size_t size) const override {
    if (size < getLength()) {
      throw std::runtime_error("buf too small.");
    }

    writeHead(buf, size);
    ByteArray::writeData(buf + 15, payloadLen);
    for (int i = 0; i < payloadLen - 4; i++) {
      ByteArray::writeData(buf + 19 + i, (uint8_t)i);
    }
  }
};


class BandwidthTestMsg : public Message {
 public:
  int payloadLen;        //                       index: 15.
  int testPacketNumber;  //                       index: 19.

  static const uint8_t msgType = (uint8_t)MessageType::bandwidthTestMsg;


  BandwidthTestMsg(uint8_t* data) : Message(data) {
    ByteArray::readData(data + 15, payloadLen);
    ByteArray::readData(data + 19, testPacketNumber);
  }

  static void update(uint8_t* data, const int& mId, const int& testNum, const int64_t& ts) {
    ByteArray::writeData(data + 3, mId);
    ByteArray::writeData(data + 7, ts);
    ByteArray::writeData(data + 19, testNum);
  }

  static void getTestNum(uint8_t* data, int& testNum) {
    ByteArray::readData(data + 19, testNum);
  }

  BandwidthTestMsg(int packetSize, int testId, int testNumber, int mid)
      : Message(msgType, testId, mid),
        payloadLen(packetSize - 15),
        testPacketNumber(testNumber) {}

  size_t getLength() const override { return headLen() + (size_t)payloadLen; }

  void getBinary(uint8_t* buf, size_t size) const override {
    if (size < getLength()) {
      throw std::runtime_error("buf too small.");
    }

    writeHead(buf, size);
    ByteArray::writeData(buf + 15, payloadLen);
    ByteArray::writeData(buf + 19, testPacketNumber);
    for (int i = 0; i < payloadLen - 8; i++) {
      ByteArray::writeData(buf + 23 + i, (uint8_t)i);
    }
  }
};


class BandwidthFinish : public Message {
 public:
  int totalTestNum;  //                   index: 15.

  static const uint8_t msgType = (uint8_t)MessageType::bandwidthFinish;

  static void getTotalPkt(uint8_t* data, int& totalPkt) {
    ByteArray::readData(data + 15, totalPkt);
  }

  BandwidthFinish(int testId, int totalNum, int mid)
      : Message(msgType, testId, mid), totalTestNum(totalNum) {}

  size_t getLength() const override { return headLen() + 4; }

  void getBinary(uint8_t* buf, size_t size) const override {
    if (size < getLength()) {
      throw std::runtime_error("buf too small.");
    }
    writeHead(buf, size);
    ByteArray::writeData(buf + 15, totalTestNum);
  }
};


class BandwidthReport : public Message {
 public:
  int jitterMicroSec;     //                   index: 15.
  int receivedPkt;        //                   index: 19.
  uint64_t transferByte;  //                   index: 23.

  static const uint8_t msgType = (uint8_t)MessageType::bandwidthReport;

  static void getTestNum(uint8_t* data, int& testNum) {
    ByteArray::readData(data + 19, testNum);
  }

  BandwidthReport(uint8_t* data) : Message(data) {
    ByteArray::readData(data + 15, jitterMicroSec);
    ByteArray::readData(data + 19, receivedPkt);
    ByteArray::readData(data + 23, transferByte);
  }

  BandwidthReport(int jitter, int recvPkt, uint64_t transBytes, int testId, int mid)
      : Message(msgType, testId, mid),
        jitterMicroSec(jitter),
        receivedPkt(recvPkt),
        transferByte(transBytes) {}

  size_t getLength() const override { return headLen() + 16; }

  void getBinary(uint8_t* buf, size_t size) const override {
    if (size < getLength()) {
      throw std::runtime_error("buf too small.");
    }
    writeHead(buf, size);
    ByteArray::writeData(buf + 15, jitterMicroSec);
    ByteArray::writeData(buf + 19, receivedPkt);
    ByteArray::writeData(buf + 23, transferByte);
  }
};