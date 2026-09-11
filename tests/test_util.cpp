#include "test_util.h"

#include <algorithm>

namespace mimir_tests {

const size_t kPatternLen = 32;

void getPattern(std::byte* pattern, int seed)
{
  for (int i = 0; i < kPatternLen; i++) {
    pattern[i] = std::byte((seed * 3162 + i * 63163) % 0xff);
  }
}

void writeData(std::byte* dest, size_t count, int seed)
{
  std::byte pattern[kPatternLen];
  getPattern(pattern, seed);

  for (int i = 0; i < count; i += kPatternLen) {
    memcpy(dest + i, pattern, std::min(count - i, kPatternLen));
  }
}

bool checkData(std::byte* src, size_t count, int seed)
{
  std::byte pattern[kPatternLen];
  getPattern(pattern, seed);

  for (int i = 0; i < count; i += kPatternLen) {
    if (memcmp(src + i, pattern, std::min(count - i, kPatternLen)) != 0) {
      return false;
    }
  }
  return true;
}

size_t alignUp(size_t val, size_t alignment)
{
  return alignDown(val - 1, alignment) + alignment;
}

size_t alignDown(size_t val, size_t alignment)
{
  return val & ~(alignment - 1);
}

}; // namespace mimir_tests
