#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <functional>

//Data types

using i8 = std::int8_t;
using u8 = std::uint8_t;

using c8 = char;
using c16 = wchar_t;

using i16 = std::int16_t;
using u16 = std::uint16_t;

using i32 = std::int32_t;
using u32 = std::uint32_t;

using i64 = std::int64_t;
using u64 = std::uint64_t;

using col16 = u16;

using f32 = float;
using f64 = double;

using usz = std::size_t;
using isz = std::ptrdiff_t;

//Classes

template<typename K, typename V>
using Map = std::unordered_map<K, V>;

template<typename A, typename B>
using Pair = std::pair<A, B>;

template<typename T>
using List = std::vector<T>;

template<typename T>
using ListIt = typename std::vector<T>::iterator;

template<typename T>
using ListItConst = typename std::vector<T>::const_iterator;

template<typename T, usz N>
using Array = std::array<T, N>;

using f32x3 = Array<f32, 3>;

using String = std::string;
using WString = std::wstring;

//Casts

constexpr u8 operator ""_u8(unsigned long long test) { return (u8)test; }
constexpr i8 operator ""_i8(unsigned long long test) { return (i8)test; }

constexpr u16 operator ""_u16(unsigned long long test) { return (u16)test; }
constexpr i16 operator ""_i16(unsigned long long test) { return (i16)test; }

constexpr u32 operator ""_u32(unsigned long long test) { return (u32)test; }
constexpr i32 operator ""_i32(unsigned long long test) { return (i32)test; }

constexpr u64 operator ""_u64(unsigned long long test) { return (u64)test; }
constexpr i64 operator ""_i64(unsigned long long test) { return (i64)test; }

constexpr usz operator ""_usz(unsigned long long test) { return (usz)test; }
constexpr isz operator ""_isz(unsigned long long test) { return (isz)test; }

constexpr f32 operator ""_f32(long double test) { return (f32)test; }
constexpr f64 operator ""_f64(long double test) { return (f64)test; }

//Max values

static constexpr usz usz_MAX = usz(-1);
static constexpr usz usz_SIZE = sizeof(usz);
static constexpr usz usz_BITSIZE = usz_SIZE << 3;

static constexpr u8 u8_MAX   = 0xFF_u8;
static constexpr u8 u8_MIN   = 0_u8;
static constexpr u16 u16_MAX = 0xFFFF_u16;
static constexpr u16 u16_MIN = 0_u16;
static constexpr u32 u32_MAX = 0xFFFFFFFF_u32;
static constexpr u32 u32_MIN = 0_u32;
static constexpr u64 u64_MAX = 0xFFFFFFFFFFFFFFFF_u64;
static constexpr u64 u64_MIN = 0_u64;
							   
static constexpr i8 i8_MAX   = 0x7F_i8;
static constexpr i8 i8_MIN   = 0x80_i8;
static constexpr i16 i16_MAX = 0x7FFF_i16;
static constexpr i16 i16_MIN = 0x8000_i16;
static constexpr i32 i32_MAX = 0x7FFFFFFF_i32;
static constexpr i32 i32_MIN = 0x80000000_i32;
static constexpr i64 i64_MAX = 0x7FFFFFFFFFFFFFFF_i64;
static constexpr i64 i64_MIN = 0x8000000000000000_i64;

//Exceptions

#ifndef NDEBUG
	#define EXCEPTION(errror) \
		throw std::runtime_error((String(errror) + " at " __FILE__ "::" + __func__ + ":" + std::to_string(__LINE__)).c_str())
#else
	#define EXCEPTION(errror) throw std::runtime_error(errror)
#endif

#define CATCH(x) catch(std::runtime_error x)

//Syntax; FINALLY(<myCleanupStuff>)
//Can be done anywhere in a func, not necessarily next to an exception
// 
//You need to call END_FINALLY at all successful returns of every FINALLY
//otherwise it will cleanup without you wanting to
//
//Every FINALLY needs to be in their own scope to function properly
//and executes at the end of the scope. This end will be reached by exceptions too
//that's the purpose

#define FINALLY(...)								\
auto scopeCleanupLambda = [&]() { __VA_ARGS__ };	\
struct Scope {										\
													\
	std::function<void()> lambda;					\
													\
	~Scope() {										\
		if(lambda)									\
			lambda();								\
	}												\
													\
} scope{ scopeCleanupLambda };

#define END_FINALLY scope.lambda = nullptr

//Simple wrapper around a pointer with size
//Can be managed or unmanaged, up to the user (if .alloc is called .dealloc must be called manually)

class Buffer {

	usz len;
	u8 *ptr;

public:

	Buffer(usz _size, u8 *_ptr);
	Buffer();

	template<typename T = u8>
	inline T *begin() const { return (T*) ptr; }

	template<typename T = u8>
	inline T *end() const { return (T*) (ptr + len); }

	template<typename T = u8>
	inline T &at(usz i = 0) const { return *(T*) (ptr + i); }

	template<typename T = u8>
	inline T &atBack(usz i = 0) const { return *(T*) (ptr - i); }

	template<typename T = u8>
	inline T *add(usz i = 0) const { return (T*) (ptr + i); }

	inline void requireSize(usz siz) const {
		if (siz > len)
			EXCEPTION("No space left in Buffer");
	}

	template<typename T = u8>
	inline T &consume() { 

		requireSize(sizeof(T));

		u8 *loc = ptr;
		addOffset(sizeof(T));

		return *(T*) loc; 
	}

	template<typename T = u8>
	inline List<T> consumeList(usz len) { 

		requireSize(sizeof(T) * len);

		u8 *loc = ptr;
		addOffset(sizeof(T) * len);

		return List<T>((T*) loc, (T*)loc + len); 
	}

	template<typename T = u8, typename ...args>
	inline void append(const T &t, const args &...arg) { 

		requireSize(sizeof(T));

		*(T*)ptr = t;
		addOffset(sizeof(T));

		if constexpr (sizeof...(args))
			append(arg...);
	}

	inline void appendBuffer(Buffer buf) {
		requireSize(buf.len);
		std::memcpy(ptr, buf.ptr, buf.len);
		addOffset(buf.len);
	}

	inline void appendString(const String &str) {
		requireSize(str.size());
		std::memcpy(ptr, str.data(), str.size());
		addOffset(str.size());
	}

	inline String consumeString(usz l) {
		requireSize(l);
		String res = String((c8*)ptr, l);
		addOffset(l);
		return res;
	}

	inline Buffer splitConsume(usz l) {
		requireSize(l);
		Buffer res = Buffer(l, ptr);
		addOffset(l);
		return res;
	}

	inline usz size() const { return len; }
	
	static Buffer alloc(usz size);
	static Buffer alloc(usz size, u8 *init);
	static Buffer allocEmpty(usz size);			//allocate and memset to 0
	void dealloc();

	//Set every byte to val
	void set(u8 val);

	Buffer offset(usz len) const;
	Buffer &addOffset(usz i);

	u8 &operator[](usz i);

	static Buffer readFile(const String &str);
	bool writeFile(const String &str);
	static Buffer copy(Buffer buf);

	inline Buffer clone() const { return copy(*this); }
};