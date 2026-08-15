//
//  block.cpp
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

#if defined(__unix__) || defined(__APPLE__) || defined(ANDROID)
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include <errno.h>
#include <cassert>
#include <cstring>

namespace mimir {

int KRENGINE_SYS_ALLOCATION_GRANULARITY;
int KRENGINE_SYS_PAGE_SIZE;

void init()
{
#if defined(_WIN32) || defined(_WIN64)

  SYSTEM_INFO winSysInfo;
  GetSystemInfo(&winSysInfo);
  KRENGINE_SYS_ALLOCATION_GRANULARITY = winSysInfo.dwAllocationGranularity;
  KRENGINE_SYS_PAGE_SIZE = winSysInfo.dwPageSize;

#elif defined(__APPLE__) || __linux__ || defined(ANDROID)

  KRENGINE_SYS_PAGE_SIZE = getpagesize();
  KRENGINE_SYS_ALLOCATION_GRANULARITY = KRENGINE_SYS_PAGE_SIZE;

#else
#error Unsupported
#endif
}

#if defined(_WIN32) || defined(_WIN64)
void ReportWindowsLastError(LPCTSTR lpszFunction)
{
  LPVOID lpMsgBuf;
  LPVOID lpDisplayBuf;
  DWORD dw = GetLastError();

  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    dw,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR)&lpMsgBuf,
    0, NULL);

  // Display the error message and exit the process

  lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
  (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
  fprintf(stderr,
    TEXT("%s failed with error %d: %s\n"),
    lpszFunction, dw, (LPCTSTR)lpMsgBuf);

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
}
#endif

} // namespace mimir
