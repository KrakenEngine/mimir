#include <gtest/gtest.h>
#include "mimir.h"

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

TEST(Mimir_Region, Uninitialized) {
  mimir::init();
  mimir::Region region;
  EXPECT_TRUE(region.getAddress() == nullptr);
  EXPECT_TRUE(region.getMaxSize() == 0);
  EXPECT_TRUE(region.getSize() == 0);
}

TEST(Mimir_Region, Resizing)
{
  mimir::init();
  mimir::Region region;

  EXPECT_TRUE(region.init(1 << 10ULL));
  EXPECT_NE(region.getAddress(), nullptr);
  EXPECT_GT(region.getMaxSize(), 0);
  EXPECT_EQ(region.getSize(), 0);
  EXPECT_TRUE(region.resize(0));
  EXPECT_EQ(region.getSize(), 0);
  EXPECT_FALSE(region.resize(1 << 20ULL));
  EXPECT_EQ(region.getSize(), 0);
  EXPECT_TRUE(region.resize(100));
  EXPECT_GE(region.getSize(), 100);
  writeData(region.getAddress(), 100, 1);
  EXPECT_TRUE(checkData(region.getAddress(), 100, 1));
  EXPECT_TRUE(region.resize(1 << 9ULL));
  writeData(region.getAddress() + 100, 900, 2);
  EXPECT_TRUE(checkData(region.getAddress(), 100, 1));
  EXPECT_TRUE(checkData(region.getAddress() + 100, 900, 2));

  EXPECT_TRUE(region.resize(1 << 10ULL));
  EXPECT_TRUE(region.resize(1 << 5ULL));
  EXPECT_TRUE(region.resize(1 << 0ULL));
  EXPECT_TRUE(region.resize(1 << 9ULL));
}
