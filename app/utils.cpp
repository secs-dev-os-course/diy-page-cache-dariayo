#include "../include/utils.h"
#include "../include/block_cache.h"

#include <cstring>
#include <iostream>

BlockCache cache(4096, 128);

int search_file(const fs::path &dir, const std::string &file_name,
                int repeat_count) {
  for (int i = 0; i < repeat_count; ++i) {
    try {
      for (const auto &entry : fs::recursive_directory_iterator(dir)) {
        if (entry.path().filename() == file_name) {
          const char *path = entry.path().c_str();

          int fd = cache.open(path);
          if (fd == -1) {
            std::cerr << "Ошибка открытия файла через кэш\n";
            return -1;
          }

          char buf[256];
          ssize_t bytes_read = cache.read(fd, buf, sizeof(buf));
          if (bytes_read > 0) {
            std::cout << "Прочитано из файла: " << std::string(buf, bytes_read)
                      << std::endl;
          } else {
            std::cerr << "Ошибка чтения файла через кэш\n";
          }

          if (cache.close(fd) == -1) {
            std::cerr << "Ошибка закрытия файла через кэш\n";
            return -1;
          }

          std::cout << "Файл найден: " << entry.path() << std::endl;
          return 0;
        }
      }
      std::cout << "Файл не найден" << std::endl;
      return 1;
    } catch (const fs::filesystem_error &e) {
      std::cerr << "Ошибка доступа: " << e.what() << std::endl;
      return 2;
    }
  }
  return 1;
}
