#include "../include/block_cache.h"
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

BlockCache::BlockCache(size_t block_size, size_t max_cache_size)
    : block_size_(block_size), max_cache_size_(max_cache_size) {}

int BlockCache::open(const char *path) {
  int fd = ::open(path, O_RDWR | O_DIRECT);
  if (fd == -1) {
    perror("Ошибка открытия файла");
    return -1;
  }
  file_descriptors_[fd] = fd;
  fd_offsets_[fd] = 0;
  return fd;
}

int BlockCache::close(int fd) {
  if (file_descriptors_.find(fd) == file_descriptors_.end()) {
    std::cerr << "Ошибка: неверный дескриптор файла\n";
    return -1;
  }

  fsync(fd);
  ::close(fd);
  file_descriptors_.erase(fd);
  fd_offsets_.erase(fd);
  return 0;
}

void *aligned_alloc(size_t alignment, size_t size) {
  void *ptr;
  if (posix_memalign(&ptr, alignment, size) != 0) {
    return nullptr;
  }
  return ptr;
}

ssize_t BlockCache::read(int fd, void *buf, size_t count) {
  if (file_descriptors_.find(fd) == file_descriptors_.end()) {
    std::cerr << "Ошибка: неверный дескриптор файла\n";
    return -1;
  }

  off_t offset = fd_offsets_[fd];
  size_t bytes_read = 0;

  while (bytes_read < count) {
    off_t block_offset = (offset / block_size_) * block_size_;
    size_t page_offset = offset % block_size_;
    size_t bytes_to_read =
        std::min(count - bytes_read, block_size_ - page_offset);

    auto it = cache_.find(block_offset);
    if (it != cache_.end()) {
      std::memcpy((char *)buf + bytes_read,
                  it->second.data.data() + page_offset, bytes_to_read);
      it->second.referenced = true;
    } else {
      CachePage page;
      page.offset = block_offset;
      page.data.resize(block_size_);

      void *aligned_buf = aligned_alloc(4096, block_size_);
      if (!aligned_buf) {
        std::cerr << "Ошибка выделения памяти\n";
        return -1;
      }

      ssize_t ret = ::pread(fd, aligned_buf, block_size_, block_offset);
      if (ret == -1) {
        free(aligned_buf);
        perror("Ошибка чтения");
        return -1;
      }

      std::memcpy(page.data.data(), aligned_buf, block_size_);
      free(aligned_buf);

      if (cache_.size() >= max_cache_size_) {
        evict_page();
      }

      page.referenced = true;
      page.modified = false;
      cache_[block_offset] = std::move(page);

      std::memcpy((char *)buf + bytes_read,
                  cache_[block_offset].data.data() + page_offset,
                  bytes_to_read);
    }

    bytes_read += bytes_to_read;
    offset += bytes_to_read;
  }

  fd_offsets_[fd] = offset;
  return bytes_read;
}

ssize_t BlockCache::write(int fd, const void *buf, size_t count) {
  if (file_descriptors_.find(fd) == file_descriptors_.end()) {
    std::cerr << "Ошибка: неверный дескриптор файла\n";
    return -1;
  }

  off_t offset = fd_offsets_[fd];
  size_t bytes_written = 0;

  while (bytes_written < count) {
    off_t block_offset = (offset / block_size_) * block_size_;
    size_t page_offset = offset % block_size_;
    size_t bytes_to_write =
        std::min(count - bytes_written, block_size_ - page_offset);

    auto it = cache_.find(block_offset);
    if (it == cache_.end()) {
      CachePage page;
      page.offset = block_offset;
      page.data.resize(block_size_);

      void *aligned_buf = aligned_alloc(4096, block_size_);
      if (!aligned_buf) {
        std::cerr << "Ошибка выделения памяти\n";
        return -1;
      }

      ssize_t ret = ::pread(fd, aligned_buf, block_size_, block_offset);
      if (ret == -1) {
        free(aligned_buf);
        perror("Ошибка чтения при записи");
        return -1;
      }

      std::memcpy(page.data.data(), aligned_buf, block_size_);
      free(aligned_buf);

      if (cache_.size() >= max_cache_size_) {
        evict_page();
      }

      page.referenced = true;
      page.modified = false;
      cache_[block_offset] = std::move(page);
    }

    std::memcpy(cache_[block_offset].data.data() + page_offset,
                (char *)buf + bytes_written, bytes_to_write);
    cache_[block_offset].modified = true;

    bytes_written += bytes_to_write;
    offset += bytes_to_write;
  }

  fd_offsets_[fd] = offset;
  return bytes_written;
}

off_t BlockCache::lseek(int fd, off_t offset, int whence) {
  if (file_descriptors_.find(fd) == file_descriptors_.end()) {
    std::cerr << "Ошибка: неверный дескриптор файла\n";
    return -1;
  }

  off_t new_offset = ::lseek(fd, offset, whence);
  if (new_offset == -1) {
    perror("Ошибка lseek");
    return -1;
  }

  fd_offsets_[fd] = new_offset;
  return new_offset;
}

int BlockCache::fsync(int fd) {
  if (file_descriptors_.find(fd) == file_descriptors_.end()) {
    std::cerr << "Ошибка: неверный дескриптор файла\n";
    return -1;
  }

  for (auto it = cache_.begin(); it != cache_.end(); ++it) {
    if (it->second.modified) {
      ssize_t ret =
          ::pwrite(fd, it->second.data.data(), block_size_, it->second.offset);
      if (ret == -1) {
        perror("Ошибка записи при fsync");
        return -1;
      }
      it->second.modified = false;
    }
  }

  return 0;
}

void BlockCache::evict_page() {
  for (auto it = cache_.begin(); it != cache_.end(); ++it) {
    if (!it->second.referenced) {
      if (it->second.modified) {
        int fd = file_descriptors_.begin()->first;
        ::pwrite(fd, it->second.data.data(), block_size_, it->second.offset);
      }
      cache_.erase(it);
      return;
    }
    it->second.referenced = false;
  }
}

