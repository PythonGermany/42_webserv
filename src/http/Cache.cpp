#include "Cache.hpp"

Cache::Cache(size_t maxSize) : _maxSize(maxSize), _size(0) {}

Cache::Cache(const Cache& rhs) { *this = rhs; }

Cache& Cache::operator=(const Cache& rhs) {
  if (this == &rhs) return *this;
  _size = rhs._size;
  _entries = rhs._entries;
  return *this;
}

Cache::~Cache() {}

bool Cache::update() {
  bool changed = false;
  std::map<std::string, data_t>::iterator it = _entries.begin();
  while (it != _entries.end()) {
    data_t& entry = it->second;
    if (entry._timestamp < std::time(NULL) - CACHE_DATA_LIFETIME) {
      _size -= it->second._data.size();
      _entries.erase(it++);
      changed = true;
    } else
      ++it;
  }
  return changed;
}

void Cache::update(const std::string& key, const std::string& data) {
  size_t newSize = _size + data.size() - _entries[key]._data.size();
  if (_size += data.size() - _entries[key]._data.size() <= _maxSize) {
    _entries[key] = (data_t){data, std::time(NULL)};
    _size = newSize;
  } else {
    _size -= _entries.at(key)._data.size();
    _entries.erase(key);
  }
}

void Cache::add(const std::string& key, const std::string& data) {
  if (!isCached(key) && _size + data.size() <= _maxSize) {
    _entries[key] = (data_t){data, std::time(NULL)};
    _size += data.size();
  }
}

std::string Cache::get(const std::string& key) {
  if (isCached(key)) return _entries.at(key)._data;
  return "";
}

bool Cache::isCached(const std::string& key) const {
  return _entries.find(key) != _entries.end();
}

bool Cache::isStale(const std::string& key, const time_t dataTime) const {
  if (isCached(key)) return _entries.at(key)._timestamp < dataTime;
  return false;
}

std::string Cache::info() const {
  std::stringstream ss;
  ss << "Size: ";
  ss << getMemorySize(_size);
  ss << "/" << getMemorySize(_maxSize);
  ss << ", Cached files: " << _entries.size();
  return ss.str();
}