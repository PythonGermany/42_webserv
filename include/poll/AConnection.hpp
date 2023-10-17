#ifndef ACONNECTION_HPP
#define ACONNECTION_HPP

#include <istream>
#include <queue>
#include <string>
#include <vector>

#include "Address.hpp"
#include "CallbackPointer.hpp"
#include "timeval.hpp"

// WEBSERV_CONFIG ----------- ACONNECTION VALUES -------------------
#define BUFFER_SIZE 65536

class AConnection : public IFileDescriptor {
 public:
  AConnection();
  AConnection(Address const &serverAddress, Address const &remoteAddress);
  AConnection(AConnection const &other);
  virtual ~AConnection();
  AConnection &operator=(AConnection const &other);

  virtual void OnCgiRecv(std::string msg) = 0;
  virtual void OnCgiError() = 0;

 protected:
  Address client;
  Address host;

  typedef enum state_e { STATUS, HEAD, BODY, CHUNK_SIZE } state_t;
  state_t _readState;

  std::string::size_type headSizeLimit;
  std::string::size_type bodySize;
  std::string readDelimiter;
  size_t _writeBufferPos;

  void setReadState(state_t readState);

  virtual void OnStatusRecv(std::string msg) = 0;
  virtual void OnHeadRecv(std::string msg) = 0;
  virtual void OnChunkSizeRecv(std::string msg) = 0;
  virtual void OnBodyRecv(std::string msg) = 0;
  void send(std::istream *msg);
  void cgiSend(std::string const &msg);
  void cgiCloseSendPipe();
  void runCGI(std::string const &program, std::vector<std::string> const &arg,
              std::vector<std::string> const &env);
  void stopReceiving();

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
  struct timeval pipeInLastTimeActive;
  void KillCgi();

  void onPollEvent(struct pollfd &pollfd, CallbackPointer *newCallbackObject,
                   struct pollfd *newPollfd);
  void onPipeInPollEvent(struct pollfd &pollfd);
  void onPipeInPollIn(struct pollfd &pollfd);
  void onPipeInNoPollEvent(struct pollfd &pollfd);
  void onPipeOutPollEvent(struct pollfd &pollfd);
  void onPipeOutPollOut(struct pollfd &pollfd);
  void onPollOut(struct pollfd &pollfd);
  void onPollIn(struct pollfd &pollfd);
  void onNoPollEvent(struct pollfd &pollfd);
  void passReadBuffer(struct pollfd &pollfd);
};

#endif  // ACONNECTION_HPP
