#include <gtest/gtest.h>
#include "mimir.h"

#include "test_util.h"

namespace mimir_tests {

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
  EXPECT_FALSE(region.init(1ULL << 50)); // Attempt to reserve too many pages
}

TEST(Region, OutOfMemory)
{
  mimir::init();
  mimir::Region region;
  EXPECT_TRUE(region.init(1ULL << 46)); // Attempt to reserve 64TB of pages
  EXPECT_FALSE(region.resize(1ULL << 60)); // Attempt to commit an extremely large number of pages
}

TEST(Region, Min1GB)
{
  mimir::init();
  mimir::Region region;
  EXPECT_TRUE(region.init(1ULL << 30)); // Allocate 1GB of pages
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

  EXPECT_TRUE(region.resize(1ULL << 4));
  EXPECT_TRUE(region.resize(1ULL << 10));
  EXPECT_TRUE(region.resize(1ULL << 5));
  EXPECT_TRUE(region.resize(1ULL << 0));
  EXPECT_TRUE(region.resize(1ULL << 9));
}

TEST(Region, Oversized)
{
  mimir::init();
  mimir::Region region;

  EXPECT_TRUE(region.init(1ULL << 4));
  size_t prevSize = region.getSize();

  EXPECT_FALSE(region.resize(1ULL << 20));
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
