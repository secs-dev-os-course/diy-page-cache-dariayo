#include "../include/block_cache.h"
#include "../include/utils.h"
#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>

void benchmark_search(const std::string &file_name, int repeat_count) {
  auto start_time = std::chrono::high_resolution_clock::now();
  int result = 0;
  for (int i = 0; i < repeat_count; ++i) {
    result = search_file("/", file_name, repeat_count);
    if (result != 0) {
      break;
    }
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end_time - start_time;
  std::cout << "Общее время выполнения: " << elapsed.count() << " секунд"
            << std::endl;

  if (result != 0) {
    std::cerr << "Ошибка поиска файла" << std::endl;
  }
}

void test_block_cache() {
  const char *test_file = "../test_file.txt";
  const size_t block_size = 4096;
  const size_t max_cache_size = 16;

  BlockCache cache(block_size, max_cache_size);

  int fd = cache.open(test_file);
  if (fd == -1) {
    std::cerr << "Ошибка открытия файла\n";
    return;
  }
  std::cout << "Файл успешно открыт: " << fd << std::endl;

  const char *write_data = "Hello!";
  ssize_t written = cache.write(fd, write_data, strlen(write_data));
  if (written == -1) {
    std::cerr << "Ошибка записи данных\n";
    cache.close(fd);
    return;
  }
  std::cout << "Данные успешно записаны: " << written << " байт\n";

  if (cache.lseek(fd, 0, SEEK_SET) == -1) {
    std::cerr << "Ошибка перемещения указателя\n";
    cache.close(fd);
    return;
  }
  std::cout << "Указатель успешно перемещен в начало файла\n";

  char read_buffer[128] = {0};
  ssize_t read_bytes = cache.read(fd, read_buffer, sizeof(read_buffer) - 1);
  if (read_bytes == -1) {
    std::cerr << "Ошибка чтения данных\n";
    cache.close(fd);
    return;
  }
  std::cout << "Данные успешно прочитаны: " << read_buffer << std::endl;

  if (cache.fsync(fd) == -1) {
    std::cerr << "Ошибка синхронизации данных\n";
    cache.close(fd);
    return;
  }
  std::cout << "Данные успешно синхронизированы с диском\n";

  if (cache.close(fd) == -1) {
    std::cerr << "Ошибка закрытия файла\n";
    return;
  }
  std::cout << "Файл успешно закрыт\n";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Использование: " << argv[0]
              << " <режим> [дополнительные аргументы]\n";
    std::cerr << "Режимы: \n";
    std::cerr << "  search <имя_файла> <количество_повторений>\n";
    std::cerr << "  test_cache\n";
    return 1;
  }

  std::string mode = argv[1];
  if (mode == "search") {
    if (argc < 4) {
      std::cerr << "Недостаточно аргументов для режима search\n";
      return 1;
    }
    std::string file_name = argv[2];
    int repeat_count = std::stoi(argv[3]);
    benchmark_search(file_name, repeat_count);
  } else if (mode == "test_cache") {
    test_block_cache();
  }

  return 0;
}
