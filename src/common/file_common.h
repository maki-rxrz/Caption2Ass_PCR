//------------------------------------------------------------------------------
// file_common.h
//------------------------------------------------------------------------------
#ifndef __FILE_COMMON_H__
#define __FILE_COMMON_H__

#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdio.h>

#ifndef fseeko

#if defined(__MINGW32__) && defined(__i386__)
#define fseeko fseeko64
#define ftello ftello64
#elif defined(_MSC_VER)
#define fseeko _fseeki64
#define ftello _ftelli64
#else
#define fseeko fseek
#define ftello ftell
#endif

#endif

#endif // __FILE_COMMON_H__
