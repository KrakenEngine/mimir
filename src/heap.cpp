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

const size_t kMinblockSize = 32;

struct TLSFBlock
{
  size_t size; // LSB: T, F: T = Last physical block, F = Free block
  TLSFBlock* prevPhys;
  TLSFBlock* prevFree;
  TLSFBlock* nextFree;
};

struct TLSFIndex
{
  uint64_t firstLevelFreeBitMap;
  uint8_t secondLevelFreeBitMap[60];
  TLSFBlock* secondLevelFreeBlocks[60 * 16];
};

uint64_t blockSizeToIndex(uint64_t size)
{
  // MSB 60 bits are the first level index
  // LSB 4 bits is the second level index
  size_t firstLevelIndex = std::bit_width(size) - 5;
  size_t secondLevelIndex = (size >> firstLevelIndex) & 0b1111;
  return (firstLevelIndex << 4) | secondLevelIndex;
}


static_assert(sizeof(TLSFBlock) == kMinblockSize);

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

  // Add the block to the index
  size_t usableSize = (block->size & ~0b11) - 16;
  
  size_t secondLevelIndex = blockSizeToIndex(usableSize);
  index->firstLevelFreeBitMap |= std::bit_floor(usableSize);
  index->secondLevelFreeBitMap[secondLevelIndex >> 4] |= secondLevelIndex & 0b1111;
  index->secondLevelFreeBlocks[secondLevelIndex] = block;

  return true;
}

// Allocate `size` bytes
std::byte* Heap::alloc(size_t size)
{
  /*
  
  HeapEmptyBlock* block = (HeapEmptyBlock*)address;
  if (block->prevBlock) {
    block->prevBlock->nextBlock = block->nextBlock;
  }
  if (block->nextBlock) {
    block->nextBlock->prevBlock = block->prevBlock;
  }

  */
  return nullptr; // not implemented
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
