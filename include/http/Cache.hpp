#ifndef CACHE_HPP
#define CACHE_HPP

#include <ctime>
#include <map>
#include <string>

#include "webserv.hpp"

class Cache {
 private:
  typedef struct data_s {
    std::string _data;
    time_t _timestamp;
  } data_t;

  size_t _maxSize;

  size_t _size;
  std::map<std::string, data_t> _entries;

 public:
  Cache(size_t maxSize = CACHE_MAX_SIZE);
  Cache(const Cache& rhs);
  Cache& operator=(const Cache& rhs);
  ~Cache();

  bool update();
  void update(const std::string& key, const std::string& data);

  void add(const std::string& key, const std::string& data);
  std::string& get(const std::string& key);

  bool isCached(const std::string& key) const;
  bool isStale(const std::string& key, const time_t dataTime) const;

  std::string info() const;
};

#endif