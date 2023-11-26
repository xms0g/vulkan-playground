#include "filesystem.h"
#include <filesystem>
#include <fstream>

std::string Filesystem::path(const char* p) {
    auto cwd = std::filesystem::current_path().string();
    cwd.erase(cwd.find_last_of('/'));
    return cwd + "/" + p;
}

std::vector<char> Filesystem::readFile(const std::string& fileName) {
    std::ifstream file{fileName, std::ios::binary | std::ios::ate};

    if (!file.is_open()) {
        throw std::runtime_error("Failed to read file: " + fileName);
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> fileBuffer(fileSize);

    file.seekg(0);

    file.read(fileBuffer.data(), fileSize);

    file.close();

    return fileBuffer;
}
