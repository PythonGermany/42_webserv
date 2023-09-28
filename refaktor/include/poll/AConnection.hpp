#ifndef ACONNECTION_HPP
#define ACONNECTION_HPP

#define BUFFER_SIZE 65536

#include <wait.h>

#include <istream>
#include <queue>
#include <string>
#include <vector>

#include "Address.hpp"
#include "CallbackPointer.hpp"
#include "ResponsePipe.hpp"
#include "timeval.hpp"
#include "webserv.hpp"

#define WAIT_FOR_HEAD std::string::npos

#include <iostream>  //tmp
class AConnection : public IFileDescriptor {
 public:
  AConnection();
  AConnection(Address const &serverAddress, Address const &remoteAddress);
  AConnection(AConnection const &other);
  virtual ~AConnection();
  AConnection &operator=(AConnection const &other);

  virtual void OnCgiRecv(std::string msg) {
    std::cout << "OnCgiRecv: " << msg << std::endl;
  };
  virtual void OnCgiError() { std::cout << "OnCgiError" << std::endl; };

 protected:
  Address client;
  Address host;
  std::string::size_type headSizeLimit;
  std::string::size_type bodySize;
  std::string headDelimiter;

  size_t _writeBufferPos;  // TODO: Pythongermany code

  virtual void OnHeadRecv(std::string msg) {
    std::cout << "OnHeadRecv(): " << msg << std::endl;
    send(new std::stringstream("hello world\n"));
    cgiSend("HELLO CGI\n");
    runCGI("cgi", std::vector<std::string>(), std::vector<std::string>());
  };
  virtual void OnBodyRecv(std::string msg) {
    std::cout << "OnBodyRecv(): " << msg << std::endl;
  };
  void send(std::istream *msg);
  void cgiSend(std::string const &msg);
  void runCGI(std::string const &program, std::vector<std::string> const &arg,
              std::vector<std::string> const &env);
  // void closeConnection();

 private:
  std::queue<std::istream *> _writeStreams;
  char _writeBuffer[BUFFER_SIZE];
  size_t _writeBufferSize;
  std::string _readBuffer;
  struct timeval lastTimeActive;
  CallbackPointer *_newCallbackObject;
  struct pollfd *_newPollfd;

  // cgi vars
  int pipeIn;
  int pipeOut;
  std::string _cgiReadBuffer;
  std::string _cgiWriteBuffer;
  pid_t _cgiPid;
  short _isListening;
  void KillCgi();

  void onPollEvent(struct pollfd &pollfd, CallbackPointer *newCallbackObject,
                   struct pollfd *newPollfd);
  void onPipeInPollEvent(struct pollfd &pollfd);
  void onPipeOutPollEvent(struct pollfd &pollfd);
  void onPollOut(struct pollfd &pollfd);
  void onPollIn(struct pollfd &pollfd);
  void onNoPollEvent(struct pollfd &pollfd);
  void passReadBuffer(struct pollfd &pollfd);
};

#endif  // ACONNECTION_HPP
