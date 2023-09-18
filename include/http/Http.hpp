#ifndef HTTP_HPP
#define HTTP_HPP

#include <iostream>

#include "AConnection.hpp"

class Http : public AConnection {
 public:
  Http(Address const &client, Address const &host);
  ~Http();

  void OnHeadRecv(std::string msg);
  void OnBodyRecv(std::string msg);
  void OnCgiRecv(std::string msg);
};

#endif  // HTTP_HPP
