//
//  arena.h
//  Kraken Engine / Mimir
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

#pragma once

#include <cstddef>

namespace mimir {
class Arena
{
public:
  Arena();
  ~Arena();
  bool init(size_t minSize = 1ULL << 24, size_t maxSize = 1ULL << 32);

  // Allocate `size` bytes
  std::byte* alloc(size_t size);

  // Allocate `size` bytes, aligned to 16 bytes and padded to next 16-byte offset.
  std::byte* allocA16(size_t size);

  // Allocate `size` bytes, aligned to 64 bytes and padded to next 64-byte offset.
  std::byte* allocA64(size_t size);

  // Reset the arena, potentially also releasing committed pages
  void reset();
private:
  std::byte* m_data;
  size_t m_minSize;
  size_t m_maxSize;
  size_t m_committedSize;
  size_t m_usedSize;

  static const size_t kWatermarkLen = 8;
  size_t m_watermark[kWatermarkLen];
}; // class Arena

} // namespace mimir
