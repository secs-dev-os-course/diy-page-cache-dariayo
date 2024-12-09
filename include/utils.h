#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

int search_file(const fs::path &dir, const std::string &file_name,
                int repeat_count);