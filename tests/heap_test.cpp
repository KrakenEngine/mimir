#include <gtest/gtest.h>
#include "mimir.h"

#include "test_util.h"

namespace mimir_tests {

TEST(Heap, InitialState)
{
  mimir::init();
  mimir::Heap heap;
  EXPECT_EQ(heap.getUsed(), 0);
}

TEST(Heap, OutOfMemory)
{
  mimir::init();
  mimir::Heap heap;
  EXPECT_TRUE(heap.init(1ULL << 24, 1ULL << 46));
  EXPECT_TRUE(heap.alloc(1ULL << 49) == nullptr);
}

}; // namespace mimir_tests
