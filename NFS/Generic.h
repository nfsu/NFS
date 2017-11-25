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

typedef enum {
	NORMAL = 0,					//Applies no 'filter' to the image, returns the basic data
	BGR5 = 0x1,					//Applies the BGR5 filter; ensures that the channel uses 5 bits instead of 8
	TILED8 = 0x10,				//Applies 'tiled8' fetch filter; expects blocks of 8x8 pixels and rearranges them
	B4 = 0x100,

	TILED8_B4 = TILED8 | B4
} TextureType;	//TextureType is a 16-bit mask that indicates: format at 1,2,4,8, fetch filter at 16,32,64,128 and layout at 256,512,1024,256

//Texture2D holds texture data and texture info
typedef struct {
	u32 size, width, height, stride;
	TextureType tt;
	u8 *data;
} Texture2D;

//PaletteTexture2D holds a palette and image
typedef struct {
	u32 width, height;
	Texture2D palette, tilemap;
} PaletteTexture2D;

///Create functions
Buffer newBuffer1(u32 size);																		//Create new empty buffer
Buffer newBuffer2(u8 *ptr, u32 size);																//Create temporary buffer
Texture2D newTexture1(u32 width, u32 height, u32 stride = 4, TextureType tt = NORMAL);				//Create new empty texture
Texture2D newTexture2(u8 *ptr, u32 width, u32 height, u32 stride = 4, TextureType tt = NORMAL);		//Create temporary texture

///Delete functions
void deleteBuffer(Buffer *b);
void deleteTexture(Texture2D *t);

///Print functions
void printBuffer(Buffer b);
void printTexture(Texture2D t, bool printContents = false);

bool isLittleEndian();														//Whether or not this system is little (or big) endian

///Getters
u16 getUShort(Buffer b, u32 offset);
u32 getUInt(Buffer b);
u32 fetchData(Texture2D t, u32 i, u32 j);									//Gets the pixel data without applying a filter
u32 getPixel(Texture2D t, u32 i, u32 j);									//Gets the pixel (with appropriate filters and stuff applied)

//Returns a new texture with the result of the 'pixel shader'
template<class T = Texture2D> Texture2D runPixelShader(u32(*func)(T, u32, u32), T t) {

	Texture2D res = { t.width * t.height * 4, t.width, t.height, 4, NORMAL, (u8*)malloc(t.width * t.height * 4) };

	for (u32 i = 0; i < t.width; ++i)
		for (u32 j = 0; j < t.height; ++j)
			((u32*)res.data)[j * t.width + i] = func(t, i, j);

	return res;
}

Texture2D convertToRGBA8(Texture2D t);										//Pixel shader for converting texture into a readable format
Texture2D convertToRGBA8(PaletteTexture2D pt2d);							//^^ convertToRGBA8({width, height, palette, texture})

///Setters
bool setUInt(Buffer b, u32 offset, u32 value);
bool setUShort(Buffer b, u32 offset, u16 value);
bool storeData(Texture2D t, u32 i, u32 j, u32 val);
bool setPixel(Texture2D t, u32 i, u32 j, u32 val);
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
bool writeTexture(Texture2D t, std::string path);