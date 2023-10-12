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
#include "webserv.hpp"

// WEBSERV_CONFIG ----------- HTTP VALUES --------------------------
#define HTTP_VERSION "HTTP/1.1"
#define HTTP_METHOD_COUNT 6
#define HTTP_METHODS \
  { "GET", "HEAD", "OPTIONS", "POST", "PUT", "DELETE" }
#define HTTP_DEFAULT_METHOD_COUNT 3
#define HTTP_DEFAULT_METHODS \
  { "GET", "HEAD", "OPTIONS" }
#define HTTP_DEFAULT_MIME "application/octet-stream"
#define MAX_CLIENT_BODY_SIZE 1048576
#define INDENT "\r\n                          "

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
  void processPutData(const std::string& data);
  void processPostData(std::string& data);
  void getPutResponse(std::string uri);
  void processOptions();
  void processDelete(std::string uri);
  void processAutoindex(std::string uri);
  void processRedirect(std::string uri);
  void processError(std::string code, std::string reason, bool close = false);
  void processCgi(std::string const& uri, File const& file,
                  std::string const& cgiPathname);

  void addIndexToPath(File& file);
  void checkResourceValidity(const File& file, std::string uri);
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
