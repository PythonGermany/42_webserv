#ifndef HTTP_HPP
#define HTTP_HPP

#include <fstream>

#include "AConnection.hpp"
#include "Request.hpp"
#include "Response.hpp"

class VirtualHost;
class Context;
class File;

// WEBSERV_CONFIG ----------- HTTP VALUES --------------------------
#define PROTOCOL "HTTP"
#define HTTP_VERSION "1.1"
#define HTTP_METHODS \
  { "GET", "HEAD", "OPTIONS", "POST", "PUT", "DELETE" }
#define HTTP_DEFAULT_METHOD_COUNT 3
#define HTTP_DEFAULT_METHODS \
  { "GET", "HEAD", "OPTIONS" }
#define HTTP_DEFAULT_MIME "application/octet-stream"
#define MAX_CLIENT_HEAD_SIZE 16384
#define MAX_CLIENT_BODY_SIZE 1048576

const std::string codes_g[] = {"200", "201", "204", "301", "400", "403", "404",
                               "405", "411", "413", "500", "501", "505"};

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

  std::string cgiProgramPathname;
  std::string cgiFilePathname;
  std::string cgiChunkedBody;

 public:
  Http(Address const& client, Address const& host);
  ~Http();

  void OnRequestRecv(std::string msg);
  void OnHeadRecv(std::string msg);
  void OnChunkSizeRecv(std::string msg);
  void OnTrailerRecv(std::string msg);
  void OnBodyRecv(std::string msg);
  void OnCgiRecv(std::string msg);
  void OnCgiError();

 private:
  void processRequest();
  void processFile(std::string uri);
  void processBodyRequest();
  void processPutData(const std::string& data);
  void processPostData(const std::string& data);
  void getPutResponse(std::string uri);
  void processOptions(std::string uri);
  void processDelete(std::string uri);
  void processAutoindex(std::string uri);
  void processRedirect(std::string uri);
  void processError(std::string code, std::string reason, bool close = false);
  void processCgi(std::string contentLength = "");

  void addIndexToPath(File& file, std::string& uri);
  void checkResourceValidity(const File& file, const std::string& uri);
  std::string getDefaultBody(std::string code, std::string reason) const;

  void sendResponse();

  std::string getAbsoluteUri(std::string uri) const;
  bool isMehodImplemented(std::string method) const;
  bool isHttpVersionValid(std::string version) const;
  bool isMethodValid() const;
  bool isBodySizeValid(size_t size) const;
  std::string getCgiPath(std::string extension) const;
  std::vector<std::string> getAllowedMethods(bool forUri = true) const;
  std::string getContextPath(std::string token, bool searchTree = false) const;
  std::string getContextArgs() const;
};

#endif
