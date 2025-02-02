// Copyright (c) 2024 Epic Games Tools
// Licensed under the MIT license (https://opensource.org/license/mit/)

#ifndef BASE_TYPES_H
#define BASE_TYPES_H

////////////////////////////////
//~ rjf: Foreign Includes

#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

////////////////////////////////
//~ rjf: Build Configuration

#if !defined(ENABLE_DEV)
# define ENABLE_DEV 0
#endif

#if !defined(SUPPLEMENT_UNIT)
# define SUPPLEMENT_UNIT 0
#endif

////////////////////////////////
//~ rjf: Codebase Keywords

#define internal      static
#define global        static
#define local_persist static

#if COMPILER_CL || (COMPILER_CLANG && OS_WINDOWS)
# pragma section(".rdata$", read)
# define read_only __declspec(allocate(".rdata$"))
#elif (COMPILER_CLANG && OS_LINUX)
# define read_only __attribute__((section(".rodata")))
#else
// NOTE(rjf): I don't know of a useful way to do this in GCC land.
// __attribute__((section(".rodata"))) looked promising, but it introduces a
// strange warning about malformed section attributes, and it doesn't look
// like writing to that section reliably produces access violations, strangely
// enough. (It does on Clang)
# define read_only
#endif

////////////////////////////////
//~ rjf: Memory Operation Macros

#define MemoryCopy(dst, src, size)    memmove((dst), (src), (size))
#define MemorySet(dst, byte, size)    memset((dst), (byte), (size))
#define MemoryCompare(a, b, size)     memcmp((a), (b), (size))
#define MemoryStrlen(ptr)             strlen(ptr)

#define MemoryCopyStruct(d,s)  MemoryCopy((d),(s),sizeof(*(d)))
#define MemoryCopyArray(d,s)   MemoryCopy((d),(s),sizeof(d))
#define MemoryCopyTyped(d,s,c) MemoryCopy((d),(s),sizeof(*(d))*(c))

#define MemoryZero(s,z)       memset((s),0,(z))
#define MemoryZeroStruct(s)   MemoryZero((s),sizeof(*(s)))
#define MemoryZeroArray(a)    MemoryZero((a),sizeof(a))
#define MemoryZeroTyped(m,c)  MemoryZero((m),sizeof(*(m))*(c))

#define MemoryMatch(a,b,z)     (MemoryCompare((a),(b),(z)) == 0)
#define MemoryMatchStruct(a,b)  MemoryMatch((a),(b),sizeof(*(a)))
#define MemoryMatchArray(a,b)   MemoryMatch((a),(b),sizeof(a))

#define MemoryRead(T,p,e)    ( ((p)+sizeof(T)<=(e))?(*(T*)(p)):(0) )
#define MemoryConsume(T,p,e) \
( ((p)+sizeof(T)<=(e))?((p)+=sizeof(T),*(T*)((p)-sizeof(T))):((p)=(e),0) )

////////////////////////////////
//~ rjf: Units

#define KB(n)  (((U64)(n)) << 10)
#define MB(n)  (((U64)(n)) << 20)
#define GB(n)  (((U64)(n)) << 30)
#define TB(n)  (((U64)(n)) << 40)
#define Thousand(n)   ((n)*1000)
#define Million(n)    ((n)*1000000)
#define Billion(n)    ((n)*1000000000)

////////////////////////////////
//~ rjf: Asserts

#if COMPILER_CL
# define Trap() __debugbreak()
#elif COMPILER_CLANG || COMPILER_GCC
# define Trap() __builtin_trap()
# else
# error "undefined trap"
#endif

#define AssertAlways(x) do{if(!(x)) {Trap();}}while(0)
#if !defined(NDEBUG)
# define Assert(x) AssertAlways(x)
#else
# define Assert(x) (void)(x)
#endif
#define AssertImplies(a,b) Assert(!(a) || b)
#define AssertIff(a,b)     Assert(!!(a) == !!(b))
#define InvalidPath        Assert(!"Invalid Path!")
#define NotImplemented     Assert(!"Not Implemented!")

#define StaticAssert(C,ID) global U8 Glue(ID,__LINE__)[(C)?1:-1]

////////////////////////////////
//~ rjf: Branch Predictor Hints

#if defined(__clang__)
# define Expect(expr, val) __builtin_expect((expr), (val))
#else
# define Expect(expr, val) (expr)
#endif

#define Likely(expr)            Expect(expr,1)
#define Unlikely(expr)          Expect(expr,0)

////////////////////////////////
//~ rjf: Misc. Helper Macros

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))

#define Stmnt(S) do{ S }while(0)

#define Stringify_(S) #S
#define Stringify(S) Stringify_(S)

#define Glue_(A,B) A##B
#define Glue(A,B) Glue_(A,B)

#define Min(A,B) ( ((A)<(B))?(A):(B) )
#define Max(A,B) ( ((A)>(B))?(A):(B) )

#define ClampTop(A,X) Min(A,X)
#define ClampBot(X,B) Max(X,B)
#define Clamp(A,X,B) ( ((X)<(A))?(A):((X)>(B))?(B):(X) )

#define PtrClampTop(A,X) ClampTop(A,X)
#define PtrClampBot(X,B) ClampBot(X,B)
#define PtrClamp(A,X,B)  Clamp(A,X,B)

#define CeilIntegerDiv(a,b) (((a) + (b) - 1)/(b))

#define Swap(T,a,b) Stmnt( T t__ = a; a = b; b = t__; )

#if ARCH_64BIT
# define IntFromPtr(ptr) ((U64)(ptr))
#elif ARCH_32BIT
# define IntFromPtr(ptr) ((U32)(ptr))
#else
# error missing ptr cast for this architecture
#endif

#define PtrFromInt(i) (void*)((U8*)0 + (i))

#define Member(T,m)                 (((T*)0)->m)
#define OffsetOf(T,m)               IntFromPtr(&Member(T,m))
#define MemberFromOffset(T,ptr,off) (T)((((U8 *)ptr)+(off)))
#define CastFromMember(T,m,ptr)     (T*)(((U8*)ptr) - OffsetOf(T,m))

#define Compose64Bit(a,b)  ((((U64)a) << 32) | ((U64)b));
#define AlignPow2(x,b)     (((x) + (b) - 1)&(~((b) - 1)))
#define AlignDownPow2(x,b) ((x)&(~((b) - 1)))
#define AlignPadPow2(x,b)  ((0-(x)) & ((b) - 1))
#define IsPow2(x)          ((x)!=0 && ((x)&((x)-1))==0)
#define IsPow2OrZero(x)    ((((x) - 1)&(x)) == 0)

#define DeferLoop(begin, end)        for(int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define DeferLoopChecked(begin, end) for(int _i_ = 2 * !(begin); (_i_ == 2 ? ((end), 0) : !_i_); _i_ += 1, (end))

#define B8  S8
#define B32 rrbool

#if LANG_CPP
# define zero_struct {}
#else
# define zero_struct {0}
#endif

#if COMPILER_MSVC && COMPILER_MSVC_YEAR < 2015
# define this_function_name "unknown"
#else
# define this_function_name __func__
#endif

#if LANG_CPP
# define C_LINKAGE_BEGIN extern "C"{
# define C_LINKAGE_END }
# define C_LINKAGE extern "C"
#else
# define C_LINKAGE_BEGIN
# define C_LINKAGE_END
# define C_LINKAGE
#endif

#if COMPILER_CL
# define thread_static __declspec(thread)
#elif COMPILER_CLANG || COMPILER_GCC
# define thread_static __thread
#endif

#if OS_WINDOWS
# define shared_function C_LINKAGE __declspec(dllexport)
#else
# define shared_function C_LINKAGE
#endif

////////////////////////////////
//~ ASAN

#if COMPILER_CL
# if defined(__SANITIZE_ADDRESS__)
#   define ASAN_ENABLED 1
#   define NO_ASAN __declspec(no_sanitize_address)
# else
#  define NO_ASAN
# endif
#elif COMPILER_CLANG
#  if defined(__has_feature)
#    if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#     define ASAN_ENABLED 1
#    endif
#  endif
# define NO_ASAN __attribute__((no_sanitize("address")))
#else
# error "NO_ASAN is not defined"
#endif

#if ASAN_ENABLED

#pragma comment(lib, "clang_rt.asan-x86_64.lib")

C_LINKAGE_BEGIN
void __asan_poison_memory_region(void const volatile *addr, size_t size);
void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
C_LINKAGE_END

# define AsanPoisonMemoryRegion(addr, size)   __asan_poison_memory_region((addr), (size))
# define AsanUnpoisonMemoryRegion(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
# define AsanPoisonMemoryRegion(addr, size)   ((void)(addr), (void)(size))
# define AsanUnpoisonMemoryRegion(addr, size) ((void)(addr), (void)(size))

#endif

////////////////////////////////
//~ rjf: Base Types

typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef int8_t   S8;
typedef int16_t  S16;
typedef int32_t  S32;
typedef int64_t  S64;
typedef S8       B8;
typedef S16      B16;
typedef S32      B32;
typedef S64      B64;
typedef float    F32;
typedef double   F64;

////////////////////////////////
//~ rjf: Large Base Types

typedef struct U128 U128;
struct U128
{
  U64 u64[2];
};

////////////////////////////////
//~ rjf: Basic Types & Spaces

typedef void VoidProc(void);

typedef enum Dimension
{
  Dimension_X,
  Dimension_Y,
  Dimension_Z,
  Dimension_W,
}
Dimension;

typedef enum Side
{
  Side_Invalid = -1,
  Side_Min,
  Side_Max,
  Side_COUNT,
}
Side;
#define side_flip(s) ((Side)(!(s)))

typedef enum Axis2
{
  Axis2_Invalid = -1,
  Axis2_X,
  Axis2_Y,
  Axis2_COUNT,
}
Axis2;
#define axis2_flip(a) ((Axis2)(!(a)))

typedef enum Corner
{
  Corner_Invalid = -1,
  Corner_00,
  Corner_01,
  Corner_10,
  Corner_11,
  Corner_COUNT
}
Corner;

////////////////////////////////
//~ rjf: Toolchain/Environment Enums

typedef enum OperatingSystem
{
  OperatingSystem_Null,
  OperatingSystem_Windows,
  OperatingSystem_Linux,
  OperatingSystem_Mac,
  OperatingSystem_COUNT,
}
OperatingSystem;

typedef enum Architecture
{
  Architecture_Null,
  Architecture_x64,
  Architecture_x86,
  Architecture_arm64,
  Architecture_arm32,
  Architecture_COUNT,
}
Architecture;

typedef enum Compiler
{
  Compiler_Null,
  Compiler_cl,
  Compiler_gcc,
  Compiler_clang,
  Compiler_COUNT,
}
Compiler;

////////////////////////////////
//~ rjf: Text 2D Coordinates & Ranges

typedef struct TxtPt TxtPt;
struct TxtPt
{
  S64 line;
  S64 column;
};

typedef struct TxtRng TxtRng;
struct TxtRng
{
  TxtPt min;
  TxtPt max;
};

////////////////////////////////
//~ NOTE(allen): Constants

global U32 sign32     = 0x80000000;
global U32 exponent32 = 0x7F800000;
global U32 mantissa32 = 0x007FFFFF;

global F32   big_golden32 = 1.61803398875f;
global F32 small_golden32 = 0.61803398875f;

global F32 pi32 = 3.1415926535897f;

global F64 machine_epsilon64 = 4.94065645841247e-324;

global U64 max_U64 = 0xffffffffffffffffull;
global U32 max_U32 = 0xffffffff;
global U16 max_U16 = 0xffff;
global U8  max_U8  = 0xff;

global S64 max_S64 = (S64)0x7fffffffffffffffull;
global S32 max_S32 = (S32)0x7fffffff;
global S16 max_S16 = (S16)0x7fff;
global S8  max_S8  =  (S8)0x7f;

global S64 min_S64 = (S64)0xffffffffffffffffull;
global S32 min_S32 = (S32)0xffffffff;
global S16 min_S16 = (S16)0xffff;
global S8  min_S8  =  (S8)0xff;

global const U32 bitmask1  = 0x00000001;
global const U32 bitmask2  = 0x00000003;
global const U32 bitmask3  = 0x00000007;
global const U32 bitmask4  = 0x0000000f;
global const U32 bitmask5  = 0x0000001f;
global const U32 bitmask6  = 0x0000003f;
global const U32 bitmask7  = 0x0000007f;
global const U32 bitmask8  = 0x000000ff;
global const U32 bitmask9  = 0x000001ff;
global const U32 bitmask10 = 0x000003ff;
global const U32 bitmask11 = 0x000007ff;
global const U32 bitmask12 = 0x00000fff;
global const U32 bitmask13 = 0x00001fff;
global const U32 bitmask14 = 0x00003fff;
global const U32 bitmask15 = 0x00007fff;
global const U32 bitmask16 = 0x0000ffff;
global const U32 bitmask17 = 0x0001ffff;
global const U32 bitmask18 = 0x0003ffff;
global const U32 bitmask19 = 0x0007ffff;
global const U32 bitmask20 = 0x000fffff;
global const U32 bitmask21 = 0x001fffff;
global const U32 bitmask22 = 0x003fffff;
global const U32 bitmask23 = 0x007fffff;
global const U32 bitmask24 = 0x00ffffff;
global const U32 bitmask25 = 0x01ffffff;
global const U32 bitmask26 = 0x03ffffff;
global const U32 bitmask27 = 0x07ffffff;
global const U32 bitmask28 = 0x0fffffff;
global const U32 bitmask29 = 0x1fffffff;
global const U32 bitmask30 = 0x3fffffff;
global const U32 bitmask31 = 0x7fffffff;
global const U32 bitmask32 = 0xffffffff;

global const U64 bitmask33 = 0x00000001ffffffffull;
global const U64 bitmask34 = 0x00000003ffffffffull;
global const U64 bitmask35 = 0x00000007ffffffffull;
global const U64 bitmask36 = 0x0000000fffffffffull;
global const U64 bitmask37 = 0x0000001fffffffffull;
global const U64 bitmask38 = 0x0000003fffffffffull;
global const U64 bitmask39 = 0x0000007fffffffffull;
global const U64 bitmask40 = 0x000000ffffffffffull;
global const U64 bitmask41 = 0x000001ffffffffffull;
global const U64 bitmask42 = 0x000003ffffffffffull;
global const U64 bitmask43 = 0x000007ffffffffffull;
global const U64 bitmask44 = 0x00000fffffffffffull;
global const U64 bitmask45 = 0x00001fffffffffffull;
global const U64 bitmask46 = 0x00003fffffffffffull;
global const U64 bitmask47 = 0x00007fffffffffffull;
global const U64 bitmask48 = 0x0000ffffffffffffull;
global const U64 bitmask49 = 0x0001ffffffffffffull;
global const U64 bitmask50 = 0x0003ffffffffffffull;
global const U64 bitmask51 = 0x0007ffffffffffffull;
global const U64 bitmask52 = 0x000fffffffffffffull;
global const U64 bitmask53 = 0x001fffffffffffffull;
global const U64 bitmask54 = 0x003fffffffffffffull;
global const U64 bitmask55 = 0x007fffffffffffffull;
global const U64 bitmask56 = 0x00ffffffffffffffull;
global const U64 bitmask57 = 0x01ffffffffffffffull;
global const U64 bitmask58 = 0x03ffffffffffffffull;
global const U64 bitmask59 = 0x07ffffffffffffffull;
global const U64 bitmask60 = 0x0fffffffffffffffull;
global const U64 bitmask61 = 0x1fffffffffffffffull;
global const U64 bitmask62 = 0x3fffffffffffffffull;
global const U64 bitmask63 = 0x7fffffffffffffffull;
global const U64 bitmask64 = 0xffffffffffffffffull;

global const U32 bit1  = (1<<0);
global const U32 bit2  = (1<<1);
global const U32 bit3  = (1<<2);
global const U32 bit4  = (1<<3);
global const U32 bit5  = (1<<4);
global const U32 bit6  = (1<<5);
global const U32 bit7  = (1<<6);
global const U32 bit8  = (1<<7);
global const U32 bit9  = (1<<8);
global const U32 bit10 = (1<<9);
global const U32 bit11 = (1<<10);
global const U32 bit12 = (1<<11);
global const U32 bit13 = (1<<12);
global const U32 bit14 = (1<<13);
global const U32 bit15 = (1<<14);
global const U32 bit16 = (1<<15);
global const U32 bit17 = (1<<16);
global const U32 bit18 = (1<<17);
global const U32 bit19 = (1<<18);
global const U32 bit20 = (1<<19);
global const U32 bit21 = (1<<20);
global const U32 bit22 = (1<<21);
global const U32 bit23 = (1<<22);
global const U32 bit24 = (1<<23);
global const U32 bit25 = (1<<24);
global const U32 bit26 = (1<<25);
global const U32 bit27 = (1<<26);
global const U32 bit28 = (1<<27);
global const U32 bit29 = (1<<28);
global const U32 bit30 = (1<<29);
global const U32 bit31 = (1<<30);
global const U32 bit32 = (1<<31);

global const U64 bit33 = (1ull<<32);
global const U64 bit34 = (1ull<<33);
global const U64 bit35 = (1ull<<34);
global const U64 bit36 = (1ull<<35);
global const U64 bit37 = (1ull<<36);
global const U64 bit38 = (1ull<<37);
global const U64 bit39 = (1ull<<38);
global const U64 bit40 = (1ull<<39);
global const U64 bit41 = (1ull<<40);
global const U64 bit42 = (1ull<<41);
global const U64 bit43 = (1ull<<42);
global const U64 bit44 = (1ull<<43);
global const U64 bit45 = (1ull<<44);
global const U64 bit46 = (1ull<<45);
global const U64 bit47 = (1ull<<46);
global const U64 bit48 = (1ull<<47);
global const U64 bit49 = (1ull<<48);
global const U64 bit50 = (1ull<<49);
global const U64 bit51 = (1ull<<50);
global const U64 bit52 = (1ull<<51);
global const U64 bit53 = (1ull<<52);
global const U64 bit54 = (1ull<<53);
global const U64 bit55 = (1ull<<54);
global const U64 bit56 = (1ull<<55);
global const U64 bit57 = (1ull<<56);
global const U64 bit58 = (1ull<<57);
global const U64 bit59 = (1ull<<58);
global const U64 bit60 = (1ull<<59);
global const U64 bit61 = (1ull<<60);
global const U64 bit62 = (1ull<<61);
global const U64 bit63 = (1ull<<62);
global const U64 bit64 = (1ull<<63);

////////////////////////////////
//~ allen: Time

typedef enum WeekDay
{
  WeekDay_Sun,
  WeekDay_Mon,
  WeekDay_Tue,
  WeekDay_Wed,
  WeekDay_Thu,
  WeekDay_Fri,
  WeekDay_Sat,
  WeekDay_COUNT,
}
WeekDay;

typedef enum Month
{
  Month_Jan,
  Month_Feb,
  Month_Mar,
  Month_Apr,
  Month_May,
  Month_Jun,
  Month_Jul,
  Month_Aug,
  Month_Sep,
  Month_Oct,
  Month_Nov,
  Month_Dec,
  Month_COUNT,
}
Month;

typedef struct DateTime DateTime;
struct DateTime
{
  U16 micro_sec; // [0,999]
  U16 msec; // [0,999]
  U16 sec;  // [0,60]
  U16 min;  // [0,59]
  U16 hour; // [0,24]
  U16 day;  // [0,30]
  union{
    WeekDay week_day;
    U32 wday;
  };
  union{
    Month month;
    U32 mon;
  };
  U32 year; // 1 = 1 CE, 0 = 1 BC
};

typedef U64 DenseTime;

////////////////////////////////
//~ allen: Files

typedef U32 FilePropertyFlags;
enum
{
  FilePropertyFlag_IsFolder = (1 << 0),
};

typedef struct FileProperties FileProperties;
struct FileProperties
{
  U64 size;
  DenseTime modified;
  DenseTime created;
  FilePropertyFlags flags;
};

////////////////////////////////
//~ Safe Casts

internal U16 safe_cast_u16(U32 x);
internal U32 safe_cast_u32(U64 x);
internal S32 safe_cast_s32(S64 x);

////////////////////////////////
//~ rjf: Large Base Type Functions

internal U128 u128_zero(void);
internal U128 u128_make(U64 v0, U64 v1);
internal B32 u128_match(U128 a, U128 b);

////////////////////////////////
//~ rjf: Bit Patterns

internal U32 u32_from_u64_saturate(U64 x);
internal U64 u64_up_to_pow2(U64 x);
internal S32 extend_sign32(U32 x, U32 size);
internal S64 extend_sign64(U64 x, U64 size);

internal F32 inf32(void);
internal F32 neg_inf32(void);

internal U16 bswap_u16(U16 x);
internal U32 bswap_u32(U32 x);
internal U64 bswap_u64(U64 x);

////////////////////////////////
//~ rjf: Enum -> Sign

internal S32 sign_from_side_S32(Side side);
internal F32 sign_from_side_F32(Side side);

////////////////////////////////
//~ rjf: Memory Functions

internal B32 memory_is_zero(void *ptr, U64 size);

////////////////////////////////
//~ rjf: Text 2D Coordinate/Range Functions

internal TxtPt txt_pt(S64 line, S64 column);
internal B32 txt_pt_match(TxtPt a, TxtPt b);
internal B32 txt_pt_less_than(TxtPt a, TxtPt b);
internal TxtPt txt_pt_min(TxtPt a, TxtPt b);
internal TxtPt txt_pt_max(TxtPt a, TxtPt b);
internal TxtRng txt_rng(TxtPt min, TxtPt max);
internal TxtRng txt_rng_intersect(TxtRng a, TxtRng b);
internal TxtRng txt_rng_union(TxtRng a, TxtRng b);

////////////////////////////////
//~ rjf: Toolchain/Environment Enum Functions

internal U64 bit_size_from_arch(Architecture arch);
internal U64 max_instruction_size_from_arch(Architecture arch);

internal OperatingSystem operating_system_from_context(void);
internal Architecture architecture_from_context(void);
internal Compiler compiler_from_context(void);

////////////////////////////////
//~ rjf: Time Functions

internal DenseTime dense_time_from_date_time(DateTime date_time);
internal DateTime date_time_from_dense_time(DenseTime time);
internal DateTime date_time_from_micro_seconds(U64 time);

////////////////////////////////
//~ rjf: Non-Fancy Ring Buffer Reads/Writes

internal U64 ring_write(U8 *ring_base, U64 ring_size, U64 ring_pos, void *src_data, U64 src_data_size);
internal U64 ring_read(U8 *ring_base, U64 ring_size, U64 ring_pos, void *dst_data, U64 read_size);
#define ring_write_struct(ring_base, ring_size, ring_pos, ptr) ring_write((ring_base), (ring_size), (ring_pos), (ptr), sizeof(*(ptr)))
#define ring_read_struct(ring_base, ring_size, ring_pos, ptr) ring_read((ring_base), (ring_size), (ring_pos), (ptr), sizeof(*(ptr)))

#endif // BASE_TYPES_H
