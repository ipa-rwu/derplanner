//
// Copyright (c) 2015 Alexander Shafranov shafranov@gmail.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef DERPLANNER_COMPILER_BASE_H_
#define DERPLANNER_COMPILER_BASE_H_

#if defined(_MSC_VER)
    #define PLNNRC_MSVC_VERSION _MSC_VER
#endif

#if defined(PLNNRC_MSVC_VERSION) && (PLNNRC_MSVC_VERSION <= 1600)
    typedef signed      __int8      int8_t;
    typedef signed      __int16     int16_t;
    typedef signed      __int32     int32_t;
    typedef signed      __int64     int64_t;
    typedef unsigned    __int8      uint8_t;
    typedef unsigned    __int16     uint16_t;
    typedef unsigned    __int32     uint32_t;
    typedef unsigned    __int64     uint64_t;
    typedef size_t uintptr_t;
#else
    #include <stdint.h>
#endif

#ifndef plnnrc_assert
    #include <assert.h>
    #define plnnrc_assert(X) assert(X)
#endif

#ifndef plnnrc_alignof
    #define plnnrc_alignof(X) __alignof(X)
#endif

#endif
