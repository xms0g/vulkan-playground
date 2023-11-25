#include "filesystem.h"
#include <filesystem>

std::string Filesystem::path(const char* p) {
    auto cwd = std::filesystem::current_path().string();
    cwd.erase(cwd.find_last_of('/'));
    return cwd + "/" + p;
}
