#pragma once
#include <stdbool.h>
#include <malloc.h>
#include <string>
#include <vector>

///Typedefs

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

//Buffer holds u8[size]
typedef struct {
	u8 *data;
	u32 size;
} Buffer;

///Create functions
Buffer newBuffer1(u32 size);																						//Create new empty buffer
Buffer newBuffer2(u8 *ptr, u32 size);																				//Create temporary buffer
																													///Delete functions
void deleteBuffer(Buffer *b);

///Print functions
void printBuffer(Buffer b);

///Getters
u16 getUShort(Buffer b, u32 offset);
u32 getUInt(Buffer b);

///Setters
bool setUInt(Buffer b, u32 offset, u32 value);
bool setUShort(Buffer b, u32 offset, u16 value);
bool copyBuffer(Buffer dest, Buffer src, u32 size, u32 offset);
void clearBuffer(Buffer b);

///Helper functions
Buffer offset(Buffer b, u32 off);

///Read functions
Buffer readFile(std::string str);

///Conversion functions
char hexChar(u8 i);
std::string toHex(u8 i);
std::string toHex16(u16 i);
std::string toHex32(u32 i);

///Write functions
bool writeBuffer(Buffer b, std::string path);