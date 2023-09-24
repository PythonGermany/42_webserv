#ifndef ACONNECTION_HPP
#define ACONNECTION_HPP

#define BUFFER_SIZE 65536

#include <string>
#include <vector>

#include "Address.hpp"
#include "IFileDescriptor.hpp"
#include "timeval.hpp"

#define WAIT_FOR_HEAD std::string::npos

class AConnection : public IFileDescriptor {
 public:
  AConnection();
  AConnection(AConnection const &other);
  virtual ~AConnection();
  AConnection &operator=(AConnection const &other);

  virtual void OnCgiRecv(std::string msg) = 0;
  virtual void OnCgiTimeout() = 0;
  void onPipePollOut(struct pollfd &pollfd);

 protected:
  Address client;
  Address host;
  std::string::size_type headSizeLimit;
  std::string::size_type bodySize;
  std::string headDelimiter;

  size_t _writeBufferPos;  // TODO: Pythongermany code

  virtual void OnHeadRecv(std::string msg) = 0;
  virtual void OnBodyRecv(std::string msg) = 0;
  void send(const std::string &msg);
  void cgiSend(std::string msg);
  void runCGI(std::string program, std::vector<std::string> &arg,
              std::vector<std::string> &env);
  void closeConnection();

 private:
  std::string _writeBuffer;
  std::string _readBuffer;
  std::string _cgiWriteBuffer;
  struct timeval lastTimeActive;

  void onPollEvent(struct pollfd &pollfd);
  void onPollOut(struct pollfd &pollfd);
  void onPollIn(struct pollfd &pollfd);
  void onNoPollEvent(struct pollfd &pollfd);
  void passReadBuffer(struct pollfd &pollfd);
};

#endif  // ACONNECTION_HPP
