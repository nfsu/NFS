#include "texture.hpp"
#include "stbi_write.h"
#include "stbi_load.h"
#include <math.h>
#include <cstring>
using namespace nfs;

Texture2D::Texture2D(u8 *ptr, u16 w, u16 h, u32 _stride, TextureType tt, TextureTiles tti): 
	data(ptr), width(w), height(h), stride(_stride), size(w * h), type(u16(tt)), tiles(u16(tti))
{
	dataSize = size * stride / (tt == TextureType::R4 ? 2 : 1);
}

Texture2D::Texture2D(): Texture2D(nullptr, 0, 0, 0) {}

Texture2D::Texture2D(NCLR &palette): 
	tiles(u16(TextureTiles::NONE)), type(u16(TextureType::BGR5)), stride(2) 
{
	TTLP &ttlp = palette.at<0>();
	width = u16(ttlp.c_colors);
	dataSize = ttlp.dataSize;
	size = dataSize / stride;
	height = u16(size / width);
	data = palette.get<0>().add();
}

static const Map<u16, u16> widthTable = {
	{ 256_u16, 16_u16 },
	{ 384_u16, 16_u16 },
	{ 512_u16, 16_u16 },
	{ 768_u16, 16_u16 },
	{ 1024_u16, 32_u16 },
	{ 1280_u16, 16_u16 },
	{ 1408_u16, 8_u16 },
	{ 1536_u16, 16_u16 },
	{ 2048_u16, 32_u16 },
	{ 2560_u16, 16_u16 },
	{ 2816_u16, 16_u16 },
	{ 4096_u16, 64_u16 },
	{ 4480_u16, 64_u16 },
	{ 5120_u16, 8_u16 },
	{ 5632_u16, 8_u16 },
	{ 6144_u16, 32_u16 },
	{ 6208_u16, 16_u16 },
	{ 6400_u16, 64_u16 },
	{ 6656_u16, 64_u16 },
	{ 7168_u16, 16_u16 },
	{ 8192_u16, 64_u16 },
	{ 9216_u16, 48_u16 },
	{ 9536_u16, 16_u16 },
	{ 10240_u16, 32_u16 },
	{ 12288_u16, 32_u16 },
	{ 14336_u16, 16_u16 },
	{ 15360_u16, 32_u16 },
	{ 16384_u16, 64_u16 },
	{ 17152_u16, 64_u16 },
	{ 19456_u16, 16_u16 },
	{ 20480_u16, 64_u16 },
	{ 24576_u16, 64_u16 },
	{ 34560_u16, 32_u16 },
	{ 32768_u16, 64_u16 },
	{ 51200_u16, 64_u16 }
};

Texture2D::Texture2D(NCGR &tilemap): tiles((u16)TextureTiles::TILED8), stride(1U) {

	RAHC &rahc = tilemap.at<0>();
	bool fourBit = rahc.tileDepth == 3;
	type = u16(fourBit ? TextureType::R4 : TextureType::INTEGER);

	dataSize = rahc.tileDataSize;
	size = dataSize * (fourBit ? 2U : 1U);

	data = tilemap.get<0>().add();

	if (rahc.isEncrypted) {

		magic = (u8*) std::malloc(dataSize);

		tiles = u16(TextureTiles::NONE);

		u32 endInd = dataSize / 2 - 1;
		u16 *beg = (u16*) data;

		u16 next = *(beg + 1);
		u32 seed = CompressionHelper::generateRandom(*beg);
		bool reverse = ((next ^ seed) & 0xFFFFU) != 0;

		i32 add = reverse ? -1 : 1;
		i32 i = reverse ? endInd : 0;

		seed = beg[i];

		while ((reverse && i >= 0) || (!reverse && i64(i) <= endInd)) {
			magic[i * 2] = u8(seed);
			magic[i * 2 + 1] = u8(seed >> 8);
			seed = CompressionHelper::generateRandom(seed);
			i += add;
		}
	}

	u16 tileWidth = rahc.tileWidth, tileHeight = rahc.tileHeight;

	if (tileWidth == u16_MAX || tileHeight == u16_MAX) {

		auto it = widthTable.find(u16(size));		//This is an estimation
		
		if (it == widthTable.end()) {
		
			//EXCEPTION("Couldn't determine width of image");
			width = u16(32 * (1 + (size >> 14)));					//Temporary
		}
		
		else width = it->second;

		if (rahc.sizeHint2 == 0)
			height = u16(size / width);

		else {

			if (!rahc.specialTiling)
				width = width * rahc.sizeHint2 / rahc.sizeHint1;

			height = u16(size / width);
		}

		if (width % 8 || height % 8)
			EXCEPTION("Width or height of the image have to be base8");

		std::printf("Texture2D Warning: NCGR size couldn't be determined, guessed %ux%u\r\n", width, height);

	} else {
		width = tileWidth * 8U;
		height = tileHeight * 8U;
	}
}

Texture2D::Texture2D(NSCR &map): tiles((u16)TextureTiles::NONE), stride(2U), type((u16)TextureType::INTEGER) {

	NRCS &nrcs = map.at<0>();

	width = nrcs.screenWidth >> 3;
	height = nrcs.screenHeight >> 3;
	dataSize = stride * width * height;
	size = width * height;

	data = map.get<0>().add();
}

Texture2D Texture2D::alloc(u16 w, u16 h, u32 stride, TextureType tt, TextureTiles tti) {

	Texture2D tex = Texture2D(nullptr, w, h, stride, tt, tti);
	tex.data = Buffer::allocEmpty(tex.getDataSize()).add();
	tex.allocated = true;

	return tex;
}

Texture2D Texture2D::readFile(const String &file, bool is16Bit) {

	int target = is16Bit ? 2 : 4;
	auto type = is16Bit ? TextureType::INTEGER16 : TextureType::ARGB8;

	int x, y, channels;
	u8 *ptr = (u8*) stbi_load(file.c_str(), &x, &y, &channels, target);

	Texture2D tex = Texture2D(ptr, u16(x), u16(y), target, type, TextureTiles::NONE);
	tex.allocated = true;
	return tex;
}

void Texture2D::writeFile(const String &file) {

	if (
		(type != u16(TextureType::ARGB8) && type != u16(TextureType::INTEGER16)) || 
		tiles != u16(TextureTiles::NONE)
	)
		EXCEPTION("Texture2D Couldn't write image; please convert to RGBA8 first");

	if (file.size() < 4 || String(file.end() - 4, file.end()) != ".png")
		EXCEPTION("Texture2D Couldn't write image; it only supports .png");

	int str = type / 8;

	if (!stbi_write_png(file.c_str(), (int)width, (int)height, str, data, str * (int)width))
		EXCEPTION("Texture2D Couldn't write image");
}

//Copying & moving

Texture2D::~Texture2D() {

	if (allocated && data) {
		std::free(data);
		data = nullptr;
	}

	if (magic) {
		std::free(magic);
		magic = nullptr;
	}
}

Texture2D::Texture2D(const Texture2D &other) {
	size = other.size;
	dataSize = other.dataSize;
	stride = other.stride;
	width = other.width;
	height = other.height;
	tiles = other.tiles;
	type = other.type;
	data = other.allocated ? Buffer::alloc(dataSize, other.data).add() : other.data;
	magic = other.magic ? Buffer::alloc(dataSize, other.magic).add() : nullptr;
	allocated = other.allocated;
}

Texture2D::Texture2D(Texture2D &&other) {
	size = other.size;
	dataSize = other.dataSize;
	stride = other.stride;
	width = other.width;
	height = other.height;
	tiles = other.tiles;
	type = other.type;
	data = other.data;
	magic = other.magic;
	allocated = other.allocated;
	other.data = nullptr;
	other.magic = nullptr;
}

Texture2D &Texture2D::operator=(const Texture2D &other) {
	size = other.size;
	dataSize = other.dataSize;
	stride = other.stride;
	width = other.width;
	height = other.height;
	tiles = other.tiles;
	type = other.type;
	data = other.allocated ? Buffer::alloc(dataSize, other.data).add() : other.data;
	magic = other.magic ? Buffer::alloc(dataSize, other.magic).add() : nullptr;
	allocated = other.allocated;
	return *this;
}

Texture2D &Texture2D::operator=(Texture2D &&other) {
	size = other.size;
	dataSize = other.dataSize;
	stride = other.stride;
	width = other.width;
	height = other.height;
	tiles = other.tiles;
	type = other.type;
	data = other.data;
	magic = other.magic;
	allocated = other.allocated;
	other.data = nullptr;
	other.magic = nullptr;
	return *this;
}

ChangeDimensionsResult Texture2D::changeDimensions(u16 w, u16 h) {

	u32 siz = u32(w) * h;

	if (size != siz) {
		std::printf("Texture2D Couldn't change dimensions; sizes didn't match\n");
		return ChangeDimensionsResult::INVALID_PIXEL_COUNT;
	}

	if (getTiles() != 0 && !(w % getTiles() == 0 && h % getTiles() == 0)) {
		std::printf("Texture2D Couldn't change dimensions; size(s) do(es)n't match the tiling\n");
		return ChangeDimensionsResult::INVALID_TILING;
	}

	width = w;
	height = h;
	return ChangeDimensionsResult::SUCCESS;
}

u32 Texture2D::getIndex(u16 i, u16 j) {

	u32 index = u32(j) * width + i;

	u32 tileCount = getTiles();

	if (!tileCount)
		return index;

	//Optimized for 8 tiled

	if (tileCount == 8) {

		u32 tileX = i >> 3;
		u32 tileY = j >> 3;
		u32 offX  = i & 7;
		u32 offY  = j & 7;

		u32 tileOff = (offY << 3) | offX;
		u32 tilePos = tileY * (width >> 3) + tileX;

		return (tilePos << 6) | tileOff;
	}
	
	//Fallback

	u32 tileX = i / tileCount;
	u32 tileY = j / tileCount;
	u32 offX = i % tileCount;
	u32 offY = j % tileCount;
	
	u32 tilePos = tileY * (width / tileCount) + tileX;
	u32 tileOff = offY * tileCount + offX;
	
	return tilePos * tileCount * tileCount + tileOff;
}

u32 Texture2D::fetch(u16 x, u16 y) {

	if (!size || !data || stride > 4) 
		return 0;

	x %= width;
	y %= height;

	u32 index = getIndex(x, y);

	bool fourBit = type == u16(TextureType::R4);

	u8 *ptr = data + ((index * stride) >> u32(fourBit));
	u32 val = 0;

	for (u32 i = 0; i < stride; ++i)
		val |= ptr[i] << (i * 8);

	if (magic && stride == 1)
		val ^= magic[index >> u32(fourBit)];

	if (fourBit && index % 2 == 0)
		val &= 0xF;

	else if (fourBit)
		val >>= 4;

	return val;
}

u32 Texture2D::read(u16 i, u16 j) {

	u32 pix = fetch(i, j);

	if (type == u16(TextureType::BGR5))
		pix = CompressionHelper::samplePixel(u16(pix));

	return pix;
}

bool Texture2D::store(u16 i, u16 j, u32 k, bool allowEncryptionHashOverrides) {

	if (!size || !data || stride > 4)
		return false;

	i %= width;
	j %= height;

	u32 index = getIndex(i, j);

	bool fourBit = type == u16(TextureType::R4);
	u32 bindex = (index * stride) >> u32(fourBit);

	u8 *ptr = data + bindex;

	//These parts are used for generating the encryption texture, so we just won't allow modifying them
	//You could fix this manually but then you'd have to regenerate the magic texture

	if (!allowEncryptionHashOverrides && magic && k != 0 && (
			bindex == 0 || bindex == 1 || bindex == 2 || bindex == 3 || 
			bindex == dataSize - 1 || bindex == dataSize - 2 || bindex == dataSize - 3 || bindex == dataSize - 4
	))
		return false;

	//

	if (fourBit)
		k = (k & 0xF) << ((index & 1) * 4);

	if (magic && stride == 1)
		k ^= magic[index >> u32(fourBit)];

	if (fourBit && !(index & 1))
		*ptr = (*ptr & 0xF0) | (k & 0xF);
	else if (fourBit)
		*ptr = (*ptr & 0xF) | (k & 0xF0);

	else std::memcpy(ptr, &k, stride);

	return true;
}

bool Texture2D::write(u16 i, u16 j, u32 k) {

	
	if (type == u16(TextureType::BGR5))
		k = CompressionHelper::storePixel(k);

	return store(i, j, k);
}

inline u32 convert(Texture2D tex, u16 i, u16 j, Texture2D t, bool fixIntegers) {
	
	u32 val = t.read(i, j);

	if ((t.getType() == TextureType::INTEGER || t.getType() == TextureType::R4) && fixIntegers) {

		if (t.getType() == TextureType::R4)
			val <<= 4;

		val = val | 0xFF000000;
	}

	return val;
}

struct ToRGBA8 {
	template<typename ...args> 
	inline u32 operator()(args &&...arg) const {
		return convert(std::forward<args>(arg)...);  
	}
};

Texture2D Texture2D::toRGBA8(bool fixIntegers) {

	if(fixIntegers)
		return fromShader(ToRGBA8(), width, height, *this, true);

	return fromShader(ToRGBA8(), width, height, *this, false);		//This optimizes the if check per pixel
}

inline u32 convertPT2D(Texture2D tex, u16 i, u16 j, Texture2D tilemap, Texture2D palette) {
	u32 val = tilemap.fetch(i, j);
	u32 color = palette.read(val & 0xF, (val >> 4) & 0xF);
	return color;
}

struct PT2D {
	template<typename ...args> 
	inline u32 operator()(args &&...arg) const { 
		return convertPT2D(std::forward<args>(arg)...);  
	}
};

Texture2D::Texture2D(NCGR &tilemap, NCLR &palette) {
	Texture2D tm = tilemap, pl = palette;
	*this = fromShader(PT2D(), tm.width, tm.height, tm, pl);
}

Texture2D::Texture2D(const Texture2D &tilemap, const Texture2D &palette) {
	*this = fromShader(PT2D(), tilemap.width, tilemap.height, tilemap, palette);
}

inline u32 convertPTT2D(Texture2D tex, u16 i, u16 j, Texture2D map, Texture2D tilemap, Texture2D palette) {

	u32 tiles = tilemap.getTiles();

	//Optimized version

	if (tiles == 8) {

		u32 tilesX = tilemap.getWidth() >> 3;

		u16 tileX = u16(i >> 3);
		u16 tileY = u16(j >> 3);
		u32 offX  = i & 7;
		u32 offY  = j & 7;

		u32 tileData = map.fetch(tileX, tileY);
		u32 tilePos = tileData & 0x3FF;
		u32 tileScl = (tileData & 0xC00) >> 10;
		u32 tilePlt = (tileData & 0xF000) >> 12;

		if (tileScl & 1)
			offX = 7 - offX;

		if (tileScl & 2)
			offY = 7 - offY;

		u16 tilemapX = u16(((tilePos % tilesX) << 3) | offX);
		u16 tilemapY = u16(((tilePos / tilesX) << 3) | offY);

		u32 val = tilemap.fetch(tilemapX, tilemapY);

		return palette.read(u16(val & 0xF), u16(((val >> 4) + tilePlt) & 0xF));
	}

	//Fallback

	u32 tilesX = tilemap.getWidth() / tiles;

	u16 tileX = u16(i / tiles);
	u16 tileY = u16(j / tiles);
	u32 offX = i % tiles;
	u32 offY = j % tiles;

	u32 tileData = map.fetch(tileX, tileY);
	u32 tilePos = tileData & 0x3FF;
	u32 tileScl = (tileData & 0xC00) >> 10;
	u32 tilePlt = (tileData & 0xF000) >> 12;

	if (tileScl & 1)
		offX = (tiles - 1) - offX;

	if (tileScl & 2)
		offY = (tiles - 1) - offY;

	u16 tilemapX = u16((tilePos % tilesX) * tiles + offX);
	u16 tilemapY = u16((tilePos / tilesX) * tiles + offY);

	u32 val = tilemap.fetch(tilemapX, tilemapY);

	return palette.read(u16(val & 0xF), u16(((val >> 4) + tilePlt) & 0xF));
}

struct PTT2D {
	template<typename ...args> 
	inline u32 operator()(args &&...arg) const { 
		return convertPTT2D(std::forward<args>(arg)...);  
	}
};

Texture2D::Texture2D(NSCR &map, NCGR &tilemap, NCLR &palette) {
	Texture2D m = map, tm = tilemap, pl = palette;
	*this = fromShader(PTT2D(), u16(m.width * tm.getTiles()), u16(m.height * tm.getTiles()), m, tm, pl);
}