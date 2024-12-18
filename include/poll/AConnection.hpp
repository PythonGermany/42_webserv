#ifndef ACONNECTION_HPP
#define ACONNECTION_HPP

#include <sys/time.h>

#include <queue>
#include <string>
#include <vector>

#include "Address.hpp"
#include "IFileDescriptor.hpp"

class CallbackPointer;

// WEBSERV_CONFIG ----------- ACONNECTION VALUES -------------------
/**
 * close connections if they are TIMEOUT milliseconds inactive
 */
#define CONNECTION_TIMEOUT 30000
#define CGI_TIMEOUT 30000
#define BUFFER_SIZE 65536

class AConnection : public IFileDescriptor {
 public:
  AConnection();
  AConnection(Address const &serverAddress, Address const &remoteAddress);
  AConnection(AConnection const &other);
  virtual ~AConnection();
  AConnection &operator=(AConnection const &other);

  static void initConnectionTimeout(int connectionTimeout);
  static void initCgiTimout(int cgiTimout);

  virtual void OnCgiRecv(std::string msg) = 0;
  virtual void OnCgiError() = 0;

 protected:
  typedef enum state_e {
    REQUEST_LINE,
    HEAD,
    BODY,
    CHUNK_SIZE,
    TRAILER
  } state_t;

  Address client;
  Address host;

  std::string::size_type _maxHeadSize;
  std::string::size_type bodySize;
  size_t _writeBufferPos;

  void setReadState(state_t readState);

  std::string const &getReadDelimiter() const;

  virtual void OnRequestRecv(std::string msg) = 0;
  virtual void OnHeadRecv(std::string msg) = 0;
  virtual void OnChunkSizeRecv(std::string msg) = 0;
  virtual void OnTrailerRecv(std::string msg) = 0;
  virtual void OnBodyRecv(std::string msg) = 0;
  void send(std::istream *msg);
  void cgiSend(std::string const &msg);
  void cgiCloseSendPipe();
  void runCGI(std::string program, std::vector<std::string> const &arg,
              std::vector<std::string> const &env);
  void stopReceiving();

 private:
  static int _connectionTimeout;
  state_t _readState;
  std::string readDelimiter;

  std::queue<std::istream *> _writeStreams;
  char _writeBuffer[BUFFER_SIZE];
  size_t _writeBufferSize;
  std::string _readBuffer;
  struct timeval lastTimeActive;
  CallbackPointer *_newCallbackObject;
  struct pollfd *_newPollfd;

  // cgi vars
  static int _cgiTimout;

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
