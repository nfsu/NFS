#include "texture.h"
#include "stbi_write.h"
#include "stbi_load.h"
#include <math.h>
#include <cstring>
using namespace nfs;

Texture2D::Texture2D(u8 *ptr, u16 w, u16 h, u32 _stride, TextureType tt, TextureTiles tti): 
	data(ptr), width(w), height(h), stride(_stride), size(w * h), dataSize(w * h * stride), type((u16)tt), tiles((u16)tti) {}

Texture2D::Texture2D() : Texture2D(nullptr, 0, 0, 0) {}

Texture2D::Texture2D(NCLR &palette): tiles((u16)TextureTiles::NONE), type((u16)TextureType::BGR5), stride(2U) {
	TTLP &ttlp = palette.at<0>();
	width = ttlp.c_colors;
	dataSize = ttlp.dataSize;
	size = dataSize / stride;
	height = size / width;
	data = palette.get<0>().ptr;
}

Texture2D::Texture2D(NCGR &tilemap): tiles((u16)TextureTiles::TILED8), stride(1U) {

	RAHC &rahc = tilemap.at<0>();
	bool fourBit = rahc.tileDepth == 3;
	type = fourBit ? (u16)TextureType::R4 : (u16)TextureType::INTEGER;

	dataSize = rahc.tileDataSize;
	size = dataSize * (fourBit ? 2U : 1U);

	data = tilemap.get<0>().ptr;

	if (rahc.isEncrypted) {

		magic = (u8*)malloc(dataSize);

		tiles = (u16)TextureTiles::NONE;

		u32 endInd = dataSize / 2 - 1;
		u16 *beg = (u16*) data, *end = beg + endInd;

		u16 next = *(beg + 1);
		u32 seed = CompressionHelper::generateRandom(*beg);
		bool reverse = ((next ^ seed) & 0xFFFFU) != 0;

		i32 add = reverse ? -1 : 1;
		i32 i = reverse ? endInd : 0;

		seed = beg[i];

		while ((reverse && i >= 0) || (!reverse && i <= endInd)) {
			u8 seed0 = seed & 0xFF;
			u8 seed1 = (seed & 0xFF00) >> 8;
			magic[i * 2] = seed0;
			magic[i * 2 + 1] = seed1;
			seed = CompressionHelper::generateRandom(seed);
			i += add;
		}

	}

	u16 tileWidth = rahc.tileWidth, tileHeight = rahc.tileHeight;

	if (tileWidth == u16_MAX || tileHeight == u16_MAX) {

		static const std::unordered_map<u16, u16> widthTable = {
			{ 256, 16 },
			{ 384, 16 },
			{ 512, 16 },
			{ 768, 16 },
			{ 1024, 32 },
			{ 1280, 16 },
			{ 1408, 8 },
			{ 1536, 16 },
			{ 2048, 32 },
			{ 2560, 16 },
			{ 2816, 16 },
			{ 4096, 64 },
			{ 4480, 64 },
			{ 5120, 8 },
			{ 5632, 8 },
			{ 6144, 32 },
			{ 6208, 16 },
			{ 6400, 64 },
			{ 6656, 64 },
			{ 7168, 16 },
			{ 8192, 64 },
			{ 9216, 48 },
			{ 9536, 16 },
			{ 10240, 32},
			{ 12288, 32 },
			{ 14336, 16 },
			{ 15360, 32 },
			{ 16384, 64 },
			{ 17152, 64 },
			{ 19456, 16 },
			{ 20480, 64 },
			{ 24576, 64 },
			{ 34560, 32 },
			{ 32768, 64 },
			{ 51200, 64 }
		};

		auto it = widthTable.find(size);
		
		if (it == widthTable.end()) {
		
			EXCEPTION("Couldn't determine width of image");
			width = 32 * (1 + (size >> 14));					//Temporary
		
		} else
			width = it->second;

		if (rahc.sizeHint2 == 0) {

			height = size / width;

		} else {

			if (!rahc.specialTiling)
				width = width * rahc.sizeHint2 / rahc.sizeHint1;

			height = size / width;

		}

		if (width % 8 != 0 || height % 8 != 0)
			EXCEPTION("Width or height of the image have to be base8");

		printf("Texture2D Warning: NCGR size couldn't be determined, guessed %ux%u\r\n", width, height);

	} else {
		width = tileWidth * 8U;
		height = tileHeight * 8U;
	}
}

Texture2D::Texture2D(NSCR &map): tiles((u16)TextureTiles::NONE), stride(2U), type((u16)TextureType::INTEGER) {

	NRCS &nrcs = map.at<0>();

	width = nrcs.screenWidth / 8U;
	height = nrcs.screenHeight / 8U;
	dataSize = stride * width * height;
	size = width * height;

	data = map.get<0>().ptr;

}

Texture2D Texture2D::alloc(u16 w, u16 h, u32 stride, TextureType tt, TextureTiles tti) {

	Texture2D tex = Texture2D(nullptr, w, h, stride, tt, tti);
	tex.data = Buffer::allocEmpty(tex.getDataSize()).ptr;
	tex.allocated = true;

	return tex;
}

Texture2D Texture2D::read(std::string file) {

	int x, y, channels;
	u8 *ptr = (u8*) stbi_load(file.c_str(), &x, &y, &channels, 4);

	Texture2D tex = Texture2D(ptr, (u16)x, (u16)y, 4U, TextureType::ARGB8, TextureTiles::NONE);
	tex.allocated = true;
	return tex;
}

void Texture2D::write(std::string file) {

	if (type != (u16)TextureType::ARGB8 || tiles != (u16)TextureTiles::NONE)
		EXCEPTION("Texture2D Couldn't write image; please convert to RGBA8 first");

	if (file.size() < 4 || std::string(file.end() - 4, file.end()) != ".png")
		EXCEPTION("Texture2D Couldn't write image; it only supports .png");

	if (!stbi_write_png(file.c_str(), (int)width, (int)height, 4, data, 4 * (int)width))
		EXCEPTION("Texture2D Couldn't write image");

}

//Copying & moving

Texture2D::~Texture2D() {

	if (allocated && data != nullptr) {
		Buffer b = { getDataSize(), data };
		b.dealloc();
		data = nullptr;
	}

	if (magic != nullptr)
		free(magic);
}

Texture2D::Texture2D(const Texture2D &other) {
	size = other.size;
	dataSize = other.dataSize;
	stride = other.stride;
	width = other.width;
	height = other.height;
	tiles = other.tiles;
	type = other.type;
	data = other.allocated ? Buffer::alloc(dataSize, other.data).ptr : other.data;
	magic = other.magic ? Buffer::alloc(dataSize, other.magic).ptr : nullptr;
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
	data = other.allocated ? Buffer::alloc(dataSize, other.data).ptr : other.data;
	magic = other.magic ? Buffer::alloc(dataSize, other.magic).ptr : nullptr;
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

TextureType Texture2D::getType() { return (TextureType)type; }

u16 Texture2D::getWidth() { return width; }
u16 Texture2D::getHeight() { return height; }
u32 Texture2D::getSize() { return size; }
u32 Texture2D::getTiles() { return tiles; }
u32 Texture2D::getDataSize() { return dataSize; }

ChangeDimensionsResult Texture2D::changeDimensions(u16 w, u16 h) {

	u32 siz = (u32)w * (u32)h;

	if (size != siz) {
		printf("Texture2D Couldn't change dimensions; sizes didn't match\n");
		return ChangeDimensionsResult::INVALID_PIXEL_COUNT;
	}

	if (getTiles() != 0 && !(w % getTiles() == 0 && h % getTiles() == 0)) {
		printf("Texture2D Couldn't change dimensions; size(s) do(es)n't match the tiling\n");
		return ChangeDimensionsResult::INVALID_TILING;
	}

	width = w;
	height = h;
	return ChangeDimensionsResult::SUCCESS;
}

u32 Texture2D::getIndex(u16 i, u16 j) {

	u32 index = j * width + i;

	u32 tiles = getTiles();

	if (tiles != 0) {

		u32 tileX = i / tiles;
		u32 tileY = j / tiles;
		u32 offX = i % tiles;
		u32 offY = j % tiles;

		u32 tilePos = tileY * (width / tiles) + tileX;
		u32 tileOff = offY * tiles + offX;

		index = tilePos * tiles * tiles + tileOff;
	}

	return index;
}

u32 Texture2D::fetch(u16 i, u16 j) {

	if (size == 0 || data == nullptr || stride > 4U) 
		return 0;

	i %= width;
	j %= height;

	u32 index = getIndex(i, j);

	bool fourBit = type == (u16)TextureType::R4;

	u8 *ptr = data + index * stride / (fourBit ? 2U : 1U);
	u32 val = 0;

	for (u32 i = 0; i < stride; ++i)
		val |= ptr[i] << (i * 8U);

	if (magic && stride == 1)
		val ^= magic[index / (fourBit ? 2U : 1U)];

	if (fourBit && index % 2U == 0U)
		val &= 0xFU;
	else if (fourBit)
		val >>= 4U;

	return val;
}

u32 Texture2D::read(u16 i, u16 j) {

	u32 pix = fetch(i, j);

	if (type == (u16)TextureType::BGR5)
		pix = CompressionHelper::samplePixel((u16)pix);

	return pix;
}

bool Texture2D::store(u16 i, u16 j, u32 k) {

	if (size == 0 || data == nullptr || stride > 4U)
		return false;

	i %= width;
	j %= height;

	u32 index = getIndex(i, j);

	bool fourBit = type == (u16)TextureType::R4;
	u32 bindex = index * stride / (fourBit ? 2U : 1U);

	u8 *ptr = data + bindex;

	if (magic && k != 0 && (
			bindex == 0 || bindex == 1 || bindex == 2 || bindex == 3 || 
			bindex == dataSize - 1 || bindex == dataSize - 2 || bindex == dataSize - 3 || bindex == dataSize - 4
	))
		return false;

	if (fourBit)
		k = (k & 0xF) << (index % 2U * 4);

	if (magic && stride == 1)
		k ^= magic[index / (fourBit ? 2U : 1U)];

	if (fourBit && index % 2U == 0U)
		*ptr = (*ptr & 0xF0U) | (k & 0xFU);
	else if (fourBit)
		*ptr = (*ptr & 0xFU) | (k & 0xF0U);
	else
		std::memcpy(ptr, &k, stride);

	return true;
}

bool Texture2D::write(u16 i, u16 j, u32 k) {

	
	if (type == (u16)TextureType::BGR5)
		k = CompressionHelper::storePixel(k);

	return store(i, j, k);
}

u32 convert(Texture2D tex, u16 i, u16 j, Texture2D t, bool fixIntegers) {
	
	u32 val = t.read(i, j);

	if ((t.getType() == TextureType::INTEGER || t.getType() == TextureType::R4) && fixIntegers)
		val = u32_MAX - val;

	return val;
}

Texture2D Texture2D::toRGBA8(bool fixIntegers) {
	return fromShader(convert, width, height, *this, fixIntegers);
}

u32 convertPT2D(Texture2D tex, u16 i, u16 j, Texture2D tilemap, Texture2D palette) {
	u32 val = tilemap.fetch(i, j);
	u32 color = palette.read(val & 0xFU, (val & 0xF0U) >> 4U);
	return color;
}

Texture2D::Texture2D(NCGR &tilemap, NCLR &palette) {
	Texture2D tm = tilemap, pl = palette;
	*this = fromShader(convertPT2D, tm.width, tm.height, tm, pl);
}

u8 *Texture2D::getPtr() { return data; }
u8 *Texture2D::getMagicTexture() { return magic; }
bool Texture2D::useEncryption() { return magic != nullptr; }

u32 convertPTT2D(Texture2D tex, u16 i, u16 j, Texture2D map, Texture2D tilemap, Texture2D palette) {

	u32 tiles = tilemap.getTiles();
	u32 tileSize = tiles * tiles * 2U;

	u32 tilesX = tilemap.getWidth() / tiles, tilesY = tilemap.getHeight() / tiles;

	u32 tileX = i / tiles;
	u32 tileY = j / tiles;
	u32 offX = i % tiles;
	u32 offY = j % tiles;

	u32 tileData = map.fetch(tileX, tileY);
	u32 tilePos = tileData & 0x3FFU;
	u32 tileScl = (tileData & 0xC00U) >> 10U;
	u32 tilePlt = (tileData & 0xF000U) >> 12U;

	if (tileScl & 0b1)
		offX = (tiles - 1) - offX;

	if (tileScl & 0b10)
		offY = (tiles - 1) - offY;

	u32 tilemapX = (tilePos % tilesX) * tiles + offX;
	u32 tilemapY = (tilePos / tilesX) * tiles + offY;

	u32 val = tilemap.fetch(tilemapX, tilemapY);
	u32 color = palette.read(val & 0xFU, ((val & 0xF0U) >> 4U) | tilePlt);
	return color;
}

Texture2D::Texture2D(NSCR &map, NCGR &tilemap, NCLR &palette) {
	Texture2D m = map, tm = tilemap, pl = palette;
	*this = fromShader(convertPTT2D, m.width * tm.getTiles(), m.height * tm.getTiles(), m, tm, pl);
}