#pragma once
#include <array>
#include <string>
#include <unordered_map>

typedef signed char i8;
typedef unsigned char u8;

typedef signed short i16;
typedef unsigned short u16;

typedef signed int i32;
typedef unsigned int u32;

typedef signed long long i64;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

#ifdef _WIN64
typedef u64 platformVar;
constexpr platformVar platformVar_MAX = 0xFFFFFFFFFFFFFFFFU;
constexpr platformVar platformVar_SIZE = 8;
constexpr platformVar platformVar_BITSIZE = 64;
#else
typedef u32 platformVar;
constexpr platformVar platformVar_MAX = 0xFFFFFFFFU;
constexpr platformVar platformVar_SIZE = 4;
constexpr platformVar platformVar_BITSIZE = 32;
#endif

constexpr u8 u8_MAX = (u8)0xFF;
constexpr u8 u8_MIN = (u8)0;
constexpr u16 u16_MAX = (u16)0xFFFF;
constexpr u16 u16_MIN = (u16)0;
constexpr u32 u32_MAX = (u32)0xFFFFFFFF;
constexpr u32 u32_MIN = (u32)0;
constexpr u64 u64_MAX = (u64)0xFFFFFFFFFFFFFFFF;
constexpr u64 u64_MIN = (u64)0;

constexpr i8 i8_MAX = (u8)0x7F;
constexpr i8 i8_MIN = (u8)0x80;
constexpr i16 i16_MAX = (u16)0x7FFF;
constexpr i16 i16_MIN = (u16)0x8000;
constexpr i32 i32_MAX = (u32)0x7FFFFFFF;
constexpr i32 i32_MIN = (u32)0x80000000;
constexpr i64 i64_MAX = (u64)0x7FFFFFFFFFFFFFFF;
constexpr i64 i64_MIN = (u64)0x8000000000000000;

struct Buffer {

	u32 size;
	u8 *ptr;

	Buffer(u32 _size, u8 *_ptr);
	Buffer();

	u8 *end();
	
	static Buffer alloc(u32 size);
	void dealloc();

	Buffer offset(u32 len);
	Buffer &addOffset(u32 i);

	u8 &operator[](u32 i);

	static Buffer read(std::string str);
	bool write(std::string str);
	static Buffer copy(Buffer buf);
};