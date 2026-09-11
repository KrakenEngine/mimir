#include <gtest/gtest.h>
#include <random>
#include "mimir.h"

#include "test_util.h"

namespace mimir_tests {

TEST(Arena, OutOfMemory)
{
  mimir::init();
  mimir::Arena arena;
  EXPECT_TRUE(arena.init(1ULL << 24, 1ULL << 46));
  EXPECT_TRUE(arena.alloc(1ULL << 49) == nullptr);
}

TEST(Arena, Overflow)
{
  mimir::init();
  mimir::Arena arena;
  EXPECT_TRUE(arena.init(128, 1024));
  EXPECT_TRUE(arena.alloc(512) != nullptr);
  EXPECT_TRUE(arena.alloc(512) != nullptr);
  EXPECT_TRUE(arena.alloc(arena.getMaxSize() - 1024 + 1) == nullptr);
}

TEST(Arena, Alignment16)
{
  mimir::init();
  mimir::Arena arena;
  EXPECT_TRUE(arena.init());

  for (int i = 0; i < 128; i++) {
    for (int j = 1; j < 128; j++) {
      EXPECT_TRUE(arena.alloc(i) != nullptr);
      for (int k = 0; k < 16; k++) {
        std::byte* addr = arena.allocA16(j);
        EXPECT_TRUE(addr != nullptr);
        EXPECT_EQ((size_t)addr & 0b1111, 0);
        EXPECT_EQ(arena.getUsed() & 0b1111, 0);
      }
      arena.reset();
    }
  }
}

TEST(Arena, Alignment64)
{
  mimir::init();
  mimir::Arena arena;
  EXPECT_TRUE(arena.init());

  for (int i = 0; i < 128; i++) {
    for (int j = 1; j < 128; j++) {
      EXPECT_TRUE(arena.alloc(i) != nullptr);
      for (int k = 0; k < 16; k++) {
        std::byte* addr = arena.allocA64(j);
        EXPECT_TRUE(addr != nullptr);
        EXPECT_EQ((size_t)addr & 0b111111, 0);
        EXPECT_EQ(arena.getUsed() & 0b111111, 0);
      }
      arena.reset();
    }
  }
}

TEST(Arena, OverflowByAlignmentA16)
{
  mimir::init();
  mimir::Arena arena;
  EXPECT_TRUE(arena.init(128, 1024));

  // Controlled test - first try with no alignment. This should pass.
  EXPECT_TRUE(arena.alloc(alignDown(arena.getMaxSize(), 16) - 15) != nullptr);
  EXPECT_TRUE(arena.alloc(arena.getMaxSize() - arena.getUsed()) != nullptr);
  arena.reset();

  // Now try with allocA16. This should fail.
  EXPECT_TRUE(arena.alloc(alignDown(arena.getMaxSize(), 16) - 15) != nullptr);
  EXPECT_TRUE(arena.allocA16(arena.getMaxSize() - arena.getUsed()) == nullptr);
}

TEST(Arena, OverflowByAlignmentA64)
{
  mimir::init();
  mimir::Arena arena;
  EXPECT_TRUE(arena.init(128, 1024));

  // Controlled test - first try with no alignment. This should pass.
  EXPECT_TRUE(arena.alloc(alignDown(arena.getMaxSize(), 64) - 63) != nullptr);
  EXPECT_TRUE(arena.alloc(arena.getMaxSize() - arena.getUsed()) != nullptr);
  arena.reset();

  // Now try with allocA64. This should fail.
  EXPECT_TRUE(arena.alloc(alignDown(arena.getMaxSize(), 64) - 63) != nullptr);
  EXPECT_TRUE(arena.allocA64(arena.getMaxSize() - arena.getUsed()) == nullptr);
}

TEST(Arena, ArenaFillAndReset)
{
  // We purposefully want to have the same random sequence each time

  const int maxAllocCount = 128;
  const int cycleCount = 128;
  std::mt19937_64 rgen(1234);
  std::uniform_int_distribution<int> count_dist(0, maxAllocCount);
  std::uniform_int_distribution<size_t> alloc_dist1(1, 256);
  std::uniform_int_distribution<size_t> alloc_dist2(1, 1024 * 256);
  std::uniform_int_distribution<size_t> alloc_dist3(1, 1024 * 1024 * 256);

  mimir::init();
  mimir::Arena arena;
  EXPECT_TRUE(arena.init(0, 1ULL << 32)); // Use up to 4GB
  size_t allocSize[maxAllocCount];
  std::byte* allocAddr[maxAllocCount];

  for (int i = 0; i < cycleCount; i++) {
    int allocCount = 0;
    if (i != 5) {
      // Make sure at least one cycle has 0 allocations
      allocCount = count_dist(rgen);
    }
    for (int j = 0; j < allocCount; j++) {
      allocSize[j] = alloc_dist1(rgen);
    }
    for (int j = 0; j < 32; j++) {
      int k = count_dist(rgen);
      if (k < allocCount) {
        allocSize[k] = alloc_dist2(rgen);
      }
    }
    
    for (int j = 0; j < 3; j++) {
      int k = count_dist(rgen);
      if (k < allocCount) {
        allocSize[k] = alloc_dist3(rgen);
      }
    }

    size_t totalSize = 0;
    for (int j = 0; j < allocCount; j++) {
      totalSize += allocSize[j];
      allocAddr[j] = arena.alloc(allocSize[j]);
      EXPECT_TRUE(allocAddr[j] != nullptr);
      if (allocAddr[j] != nullptr) {
        writeData(allocAddr[j], allocSize[j], j);
      }
    }

    for (int j = 0; j < allocCount; j++) {
      if (allocAddr[j] != nullptr) {
        EXPECT_TRUE(checkData(allocAddr[j], allocSize[j], j));
      }
    }

    EXPECT_EQ(arena.getUsed(), totalSize);

    arena.reset();
    EXPECT_EQ(arena.getUsed(), 0);
  }
}

}; // namespace mimir_tests
