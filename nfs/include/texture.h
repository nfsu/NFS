#pragma once

#include "ntypes.h"

namespace nfs {

	enum class TextureType : u16 {
		ARGB8, BGR5, R4, INTEGER
	};

	enum class TextureTiles : u16 {
		NONE = 1, TILED8 = 8
	};

	enum class ChangeDimensionsResult {
		SUCCESS = 0,
		INVALID_PIXEL_COUNT = 1,
		INVALID_TILING = 2
	};

	class Texture2D {

	public:

		Texture2D();
		~Texture2D();
		Texture2D(u8 *ptr, u16 w, u16 h, u32 stride, TextureType tt = TextureType::ARGB8, TextureTiles tti = TextureTiles::NONE);
		Texture2D(NCLR &palette);
		Texture2D(NCGR &tilemap);
		Texture2D(NSCR &map);

		Texture2D(const Texture2D &other);
		Texture2D(Texture2D &&other);

		Texture2D &operator=(const Texture2D &other);
		Texture2D &operator=(Texture2D &&other);

		//Allocate a new Texture2D from palette and tilemap
		Texture2D(NCGR &tilemap, NCLR &palette);

		//Allocate a new Texture2D from palette, tilemap and map
		Texture2D(NSCR &map, NCGR &tilemap, NCLR &palette);

		//Allocate new texture
		static Texture2D alloc(u16 w, u16 h, u32 stride, TextureType tt = TextureType::ARGB8, TextureTiles tti = TextureTiles::NONE);

		//Allocate new texture, created through cpu 'pixel shader'
		//Texture2D will always be a RGBA8 Normal image
		template<typename ...args>
		static Texture2D fromShader(u32(*f)(Texture2D, u16, u16, args...), u16 w, u16 h, args... arg);

		//Allocate new texture; read from the file
		//Texture2D will always be a RGBA8 Normal image
		static Texture2D read(std::string file);

		//Write to disk (only if Texture2D is RGBA8 Normal image)
		void write(std::string file);

		//Allocate new texture and convert to RGBA8
		//If fixIntegers is true, it will inverse integer formats, so it will range from white opaque to black invisible
		//It calls 'read' on every pixel, to convert it to RGBA8
		Texture2D toRGBA8(bool fixIntegers = true);

		TextureType getType();

		u8 getBitsPerPixel();
		u16 getWidth();
		u16 getHeight();
		u32 getSize();
		u32 getTiles();
		u32 getDataSize();
		bool useEncryption();
		
		//Change the dimensions of a texture; the only requirement is that w*h = this->size
		//If the texture is tiled w % tiles and h % tiles have to be equal to 0
		//Example for 64x48 texture (tiled8):
		//changeDimensions(48, 64) <- SUCCESS (0)
		//changeDimensions(64, 64) <- INVALID_PIXEL_COUNT (1) (64x64 != 64x48)
		//changeDimensions(32, 96) <- SUCCESS (0)
		//changeDimensions(3, 1024) <- INVALID_TILING (2) (3x1024 is same size, but tiles don't match (3 % 8 = 3 != 0)
		ChangeDimensionsResult changeDimensions(u16 w, u16 h);

		//Fetch the data for the pixel (if it uses BGR555 it will fetch 2 bytes, RGBA8 will fetch 4 bytes)
		u32 fetch(u16 i, u16 j);

		//Fetch the data and convert to RGBA8 (if possible); BGR555 -> ARGB8 for example
		u32 read(u16 i, u16 j);

		//Store the data for the pixel (converts it to the appropriate type first)
		bool store(u16 i, u16 j, u32 k);

		//Convert pixel to image format and then store it
		bool write(u16 i, u16 j, u32 k);

		u32 getIndex(u16 i, u16 j);

		u8 *getPtr();
		u8 *getMagicTexture();

	private:

		u32 size, dataSize, stride;
		u16 width, height;
		u16 tiles, type;

		u8 *data, *magic = nullptr /* Decryption buffer */;
		bool allocated = false;

	};


	template<typename ...args>
	Texture2D Texture2D::fromShader(u32(*f)(Texture2D, u16, u16, args...), u16 w, u16 h, args... arg) {

		Texture2D tex = Texture2D::alloc(w, h, 4U);

		for (u32 i = 0; i < (u32)w * (u32)h; ++i)
			((u32*)tex.data)[i] = f(tex, i % w, i / w, arg...);

		return tex;
	}

}