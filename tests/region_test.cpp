#include <gtest/gtest.h>
#include "mimir.h"

namespace mimir_tests {

void writeData(std::byte* dest, size_t count, int seed)
{
  for (int i = 0; i < count; i++) {
    dest[i] = std::byte((seed * 3162 + i * 63163) % 0xff);
  }
}

bool checkData(std::byte* src, size_t count, int seed)
{
  for (int i = 0; i < count; i++) {
    if (src[i] != std::byte((seed * 3162 + i * 63163) % 0xff)) {
      return false;
    }
  }
  return true;
}

TEST(Region, InitialState)
{
  mimir::init();
  mimir::Region region;
  EXPECT_TRUE(region.getAddress() == nullptr);
  EXPECT_TRUE(region.getMaxSize() == 0);
  EXPECT_TRUE(region.getSize() == 0);
}

TEST(Region, TooBig)
{
  mimir::init();
  mimir::Region region;
  EXPECT_FALSE(region.init(1 << 44ULL)); // Attempt to allocate 16TB of pages
}

TEST(Region, Min1GB)
{
  mimir::init();
  mimir::Region region;
  EXPECT_TRUE(region.init(1 << 30ULL)); // Allocate 1GB of pages
}

TEST(Region, Resizing)
{
  mimir::init();
  mimir::Region region;

  EXPECT_TRUE(region.init(1 << 10ULL));
  EXPECT_NE(region.getAddress(), nullptr);
  EXPECT_GT(region.getMaxSize(), 0);
  EXPECT_EQ(region.getSize(), 0);
  EXPECT_TRUE(region.resize(0));
  EXPECT_EQ(region.getSize(), 0);

  EXPECT_TRUE(region.resize(1 << 4ULL));
  EXPECT_TRUE(region.resize(1 << 10ULL));
  EXPECT_TRUE(region.resize(1 << 5ULL));
  EXPECT_TRUE(region.resize(1 << 0ULL));
  EXPECT_TRUE(region.resize(1 << 9ULL));
}

TEST(Region, OverSized)
{
  mimir::init();
  mimir::Region region;

  EXPECT_TRUE(region.resize(1 << 4ULL));
  size_t prevSize = region.getSize();

  EXPECT_FALSE(region.resize(1 << 20ULL));
  EXPECT_EQ(region.getSize(), prevSize);
}

TEST(Region, WriteData)
{
  mimir::init();
  mimir::Region region;

  EXPECT_TRUE(region.init());
  EXPECT_TRUE(region.resize(100));
  EXPECT_GE(region.getSize(), 100);
  writeData(region.getAddress(), 100, 1);
  EXPECT_TRUE(checkData(region.getAddress(), 100, 1));
  EXPECT_TRUE(region.resize(1 << 9ULL));
  writeData(region.getAddress() + 100, 900, 2);
  EXPECT_TRUE(checkData(region.getAddress(), 100, 1));
  EXPECT_TRUE(checkData(region.getAddress() + 100, 900, 2));
}

}; // namespace mimir_tests
