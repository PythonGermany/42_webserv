#ifndef HTTP_HPP
#define HTTP_HPP

#include <sstream>

#include "AConnection.hpp"
#include "Cache.hpp"
#include "Log.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "VirtualHost.hpp"

class Http : public AConnection {
  static Cache _cache;

  Request _request;
  Response _response;
  VirtualHost* _virtualHost;
  Context* _context;
  std::string _uri;
  bool _responseReady;

  std::string _log;

 public:
  Http(Address const& client, Address const& host);
  ~Http();

  static bool updateCache();
  static const Cache& getCache();

  void OnHeadRecv(std::string msg);
  void OnBodyRecv(std::string msg);
  void OnCgiRecv(std::string msg);
  void OnCgiError();

 private:
  Response& processRequest();
  Response& processFile(std::string uri);
  Response& processUploadHead();
  Response& processUploadBody(std::string uri);
  Response& processOptions();
  Response& processDelete(std::string uri);
  Response& processAutoindex(std::string uri);
  Response& processRedirect(std::string uri);
  Response& processError(std::string code, std::string reason);

  std::string getDefaultBody(std::string code, std::string reason) const;

  void sendResponse();

  std::string getAbsoluteUri(std::string uri) const;

  bool isMehodImplemented(std::string method) const;
  bool isMethodValid();
  std::vector<std::string> getAllowedMethods(bool forUri = true) const;
  std::string getContextPath(std::string token, bool searchTree = false) const;
  std::string getContextArgs() const;
};

#endif
