//
//  heap.cpp
//  Kraken Engine
//
//  Copyright 2026 Kearwood Gilbert. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED BY KEARWOOD GILBERT ''AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KEARWOOD GILBERT OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Kearwood Gilbert.
//

#include "../include/mimir.h"
#include "mimir_impl.h"

#include <cassert>
#include <cstring>
#include <bit>

namespace mimir {

// Two-Level Segregated Fit (TLSF): http://www.gii.upv.es/tlsf/files/papers/ecrts04_tlsf.pdf

const size_t kMinblockSize = 16;

struct TLSFBlock
{
  size_t size; // LSB: T, F: T = Last physical block, F = Free block
  TLSFBlock* prevPhys;
  TLSFBlock* prevFree;
  TLSFBlock* nextFree;
};
static_assert(sizeof(TLSFBlock) == 32);

struct TLSFIndex
{
  uint64_t firstLevelFreeBitMap;
  uint8_t secondLevelFreeBitMap[60];
  TLSFBlock* secondLevelFreeBlocks[60][16];
};

// First Level Ranges:
// 0:  0b00000000010000  (    1 ~   16 )
// 1:  0b00000000100000  (   17 ~   32 )
// 2:  0b00000001000000  (   33 ~   64 )
// 3:  0b00000010000000  (   65 ~  128 )
// 4:  0b00000100000000  (  129 ~  256 )
// 5:  0b00001000000000  (  257 ~  512 )
// 6:  0b00010000000000  (  513 ~ 1024 )
// 7:  0b00100000000000  ( 1025 ~ 2048 )
//
// bit_width(15): 4    bit_floor(15): 0b000001000
// bit_width(16): 5    bit_floor(16): 0b000010000
// bit_width(31): 5    bit_floor(31): 0b000010000
// bit_width(32): 6    bit_floor(32): 0b000100000
// bit_width(63): 6    bit_floor(63): 0b000100000
// bit_width(64): 7    bit_floor(64): 0b001000000

/*
uint64_t blockSizeToIndex(uint64_t size)
{
  // MSB 60 bits are the first level index
  // LSB 4 bits is the second level index

  // size              0b0000001101011010000
  // bit_width         13
  // bit_width - 5     8
  // size >> 8         0b0000000000000011010
  //   & 0b1111        0b1010

  size_t firstLevelIndex = std::bit_width(size) - 5;
  size_t secondLevelIndex = (size >> firstLevelIndex) & 0b1111;
  return (firstLevelIndex << 4) | secondLevelIndex;
}
*/

Heap::Heap()
{
}

Heap::~Heap()
{
}

bool Heap::init(size_t maxSize)
{
  if (!m_region.init(maxSize)) {
    return false;
  }
  if (!m_region.resize(sizeof(TLSFIndex) + 16)) {
    return false;
  }
  TLSFIndex* index = (TLSFIndex*)m_region.getAddress();
  memset(index, 0, sizeof(TLSFIndex));

  // Start with one free block, filling the entire Region
  TLSFBlock* block = (TLSFBlock*)(m_region.getAddress() + sizeof(TLSFIndex));
  block->nextFree = nullptr;
  block->prevFree = nullptr;
  block->size = m_region.getMaxSize() - sizeof(TLSFIndex);
  block->size |= 0b11; // T=1: Last Block, F=1: Free Block

  insertFreeBlock(block);

  return true;
}

void Heap::insertFreeBlock(TLSFBlock* block)
{
  // Add the block to the index

  TLSFIndex* index = (TLSFIndex*)m_region.getAddress();

  size_t usableSize = (block->size & ~0b11) - 16;

  size_t firstLevelIndex = std::bit_width(usableSize) - 5;
  size_t secondLevelIndex = (usableSize >> firstLevelIndex) & 0b1111ULL;
  index->firstLevelFreeBitMap |= std::bit_floor(usableSize);
  index->secondLevelFreeBitMap[firstLevelIndex] |= secondLevelIndex & 0b1111ULL;
  TLSFBlock* prevFirstFreeBlock = index->secondLevelFreeBlocks[firstLevelIndex][secondLevelIndex];
  index->secondLevelFreeBlocks[firstLevelIndex][secondLevelIndex] = block;

  block->prevFree = nullptr;
  if (prevFirstFreeBlock) {
    block->nextFree = prevFirstFreeBlock;
    prevFirstFreeBlock->prevFree = block;
  }
}

void Heap::removeFreeBlock(TLSFBlock* block)
{
  // Remove the block from the index
  TLSFIndex* index = (TLSFIndex*)m_region.getAddress();

  size_t usableSize = (block->size & ~0b11) - 16;

  size_t firstLevel = std::bit_width(usableSize);
  size_t firstLevelIndex = firstLevel - 5;
  size_t secondLevelIndex = (usableSize >> (firstLevel - 5)) & 0b1111ULL;

  if (block->prevFree != nullptr) {
    // Link the neighboring free blocks together
    block->prevFree = block->nextFree;
    if (block->nextFree) {
      block->nextFree = block->prevFree;
    }
  } else {
    // This block was the first for this level
    index->secondLevelFreeBlocks[firstLevelIndex][secondLevelIndex] = block->nextFree;

    if (block->nextFree == nullptr) {
      // We have removed all the blocks at this level.

      // Update second level bitmask.
      index->secondLevelFreeBitMap[firstLevelIndex] &= ~(1ULL << secondLevelIndex);

      // Check if there are any remaining free blocks within the first level
      if (index->secondLevelFreeBitMap[firstLevelIndex] == 0) {

        // This was the last one.  Clear the first level bit as well.
        index->firstLevelFreeBitMap &= ~(1ULL << firstLevel);
      }
    }
  }

  if (block->size & 0b10ULL) {
    // This block was the last in physical order.
    if (block->prevPhys) {
      // Update the prior block in physical order to mark it as the last block.
      block->prevPhys->size &= 0b10ULL;
    }
  }
}

// Allocate `size` bytes
std::byte* Heap::alloc(size_t size)
{
  if (size < kMinblockSize) {
    size = kMinblockSize;
  }
  TLSFIndex* index = (TLSFIndex*)m_region.getAddress();

  // size:           0b0000000000010000 (16)
  // bit_ceil(size): 0b0000000000010000
  // - 1 ...         0b0000000000001111
  // ~ ...           0b1111111111110000

  // size:           0b0000001110011101 (925)
  // bit_ceil(size): 0b0000010000000000
  // - 1 ...         0b0000001111111111
  // ~ ...           0b1111110000000000

  uint64_t possibleFirstLevels = ~(std::bit_floor(size) - 1);
  uint64_t freeFirstLevels = index->firstLevelFreeBitMap & possibleFirstLevels;
  uint64_t selectedFirstLevel = std::countr_zero(freeFirstLevels);
  if (selectedFirstLevel == 64) {
    // No first level with free blocks of sufficient size.
    return nullptr;
  }

  uint64_t firstLevelBufferMinSize = 1ULL << selectedFirstLevel;
  uint64_t secondLevelMask = 0b1111ULL;
  if (size > firstLevelBufferMinSize) {
    uint64_t delta = size - firstLevelBufferMinSize;
    secondLevelMask = (delta >> (selectedFirstLevel - 4)) & 0b1111ULL;
  }
  uint64_t possibleSecondLevels = secondLevelMask & index->secondLevelFreeBitMap[selectedFirstLevel - 4];
  uint64_t selectedSecondLevel = std::countr_zero(possibleSecondLevels);
  if (selectedSecondLevel == 64) {
    // No free blocks available at this first level.
  
    // Select the next highest available first level.
    selectedFirstLevel = std::countr_zero(freeFirstLevels & ~(1ULL << selectedFirstLevel));
    if (selectedFirstLevel == 64) {
      // No other first level with free blocks of sufficient size.
      return nullptr;
    }

    // Any buffer at the second level will fit this allocation.
    selectedSecondLevel = std::countr_zero(index->secondLevelFreeBitMap[selectedFirstLevel - 4]);
  }

  // Take the first free block
  TLSFBlock* block = index->secondLevelFreeBlocks[selectedFirstLevel - 4][selectedSecondLevel];

  size_t oldBlockSize = block->size & ~0b11ULL;
  bool oldBlockWasLastPhysBlock = (block->size & ~0b10ULL) != 0;

  TLSFBlock* prevFreeBlock = block->prevFree;
  TLSFBlock* nextFreeBlock = block->nextFree;
  TLSFBlock* prevPhysBlock = block->prevPhys;
  TLSFBlock* nextPhysBlock = nullptr;
  if (!oldBlockWasLastPhysBlock) {
    nextPhysBlock = (TLSFBlock*)(((std::byte*)block) + oldBlockSize + 16);
  }

  // The next free block at this level replaces this block in the index.
  index->secondLevelFreeBlocks[selectedFirstLevel - 4][selectedSecondLevel] = nextFreeBlock;

  if (nextFreeBlock == nullptr) {
    // This was the last free block at this level.
    // Update the bitmap for the second level...
    index->secondLevelFreeBitMap[selectedFirstLevel - 4] &= ~(1ULL << selectedSecondLevel);

    // Check if there are any remaining free blocks within the first level
    if (index->secondLevelFreeBitMap[selectedFirstLevel - 4] == 0) {
      
      // This was the last one.  Clear the first level bit as well.
      index->firstLevelFreeBitMap &= ~(1ULL << selectedFirstLevel);
    }
  } else {
    // The next free block is now the first free block at this level.
    nextFreeBlock->prevFree = nullptr;
  }

  if (nextPhysBlock) {
    // Link the next physical block to this one.
    nextPhysBlock->prevPhys = block;
  } else {
    // This is the last physical block, so set the last block bit.
    block->size &= 0b10ULL;
  }

  block->size = size;
  return (std::byte*)block + 16;
}

// Allocate `size` bytes, aligned to 16 bytes and padded to next 16-byte offset.
std::byte* Heap::allocA16(size_t size)
{
  return nullptr; // not implemented
}

// Allocate `size` bytes, aligned to 64 bytes and padded to next 64-byte offset.
std::byte* Heap::allocA64(size_t size)
{
  return nullptr; // not implemented
}

// Free the allocation at `address`
void Heap::free(std::byte* address)
{
}

} // namespace mimir
