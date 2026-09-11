#pragma once

#include <cstddef>

namespace mimir_tests {

void writeData(std::byte* dest, size_t count, int seed);
bool checkData(std::byte* src, size_t count, int seed);
size_t alignUp(size_t val, size_t alignment);
size_t alignDown(size_t val, size_t alignment);

}; // namespace mimir_tests
