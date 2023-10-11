#ifndef HTTP_HPP
#define HTTP_HPP

#include <fstream>
#include <sstream>

#include "AConnection.hpp"
#include "File.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "VirtualHost.hpp"
#include "global.hpp"

class Http : public AConnection {
  Request _request;
  Response _response;
  VirtualHost* _virtualHost;
  Context* _context;
  std::string _uri;

  std::ofstream _file;
  bool _newFile;
  size_t _expectedBodySize;
  size_t _currBodySize;

  std::string _log;

 public:
  Http(Address const& client, Address const& host);
  ~Http();

  void OnHeadRecv(std::string msg);
  void OnChunkSizeRecv(std::string msg);
  void OnBodyRecv(std::string msg);
  void OnCgiRecv(std::string msg);
  void OnCgiError();

 private:
  void processRequest();
  void processFile(std::string uri);
  void processBodyRequest();
  void processPutData(std::string uri, std::string& data);
  void getPutResponse(std::string uri);
  void processOptions();
  void processDelete(std::string uri);
  void processAutoindex(std::string uri);
  void processRedirect(std::string uri);
  void processError(std::string code, std::string reason, bool close = false);

  std::string getDefaultBody(std::string code, std::string reason) const;

  void sendResponse();

  std::string getAbsoluteUri(std::string uri) const;

  bool isMehodImplemented(std::string method) const;
  bool isMethodValid();
  bool isBodySizeValid(size_t size) const;
  bool isCgiExtension(std::string extension) const;
  std::vector<std::string> getAllowedMethods(bool forUri = true) const;
  std::string getContextPath(std::string token, bool searchTree = false) const;
  std::string getContextArgs() const;
};

#endif
