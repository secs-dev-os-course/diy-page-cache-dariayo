#pragma once

#include <iostream>
#include <unistd.h>
#include <unordered_map>
#include <vector>

struct CachePage {
  off_t offset;
  std::vector<char> data;
  bool modified;
  bool referenced;
};

class BlockCache {
public:
  explicit BlockCache(size_t block_size, size_t max_cache_size);

  int open(const char *path);
  int close(int fd);
  ssize_t read(int fd, void *buf, size_t count);
  ssize_t write(int fd, const void *buf, size_t count);
  off_t lseek(int fd, off_t offset, int whence);
  int fsync(int fd);

private:
  size_t block_size_;
  size_t max_cache_size_;
  std::unordered_map<off_t, CachePage> cache_;
  std::unordered_map<int, off_t> fd_offsets_;
  std::unordered_map<int, int> file_descriptors_;

  void evict_page();
};
