#pragma once

#include <string>
#include <vector>

namespace Filesystem {

std::string path(const char* p);
std::vector<char> readFile(const std::string& fileName);

}
