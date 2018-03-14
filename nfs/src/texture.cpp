#include "texture.h"
#include "stbi_write.h"
#include "stbi_load.h"
using namespace nfs;

Texture2D::Texture2D(u8 *ptr, u16 w, u16 h, u32 _stride, TextureType tt, TextureTiles tti): data(ptr), width(w), height(h), stride(_stride), size(w * h), dataSize(w * h * stride), type((u16)tt), flags((u16)tti) {}

Texture2D::Texture2D(NCLR &palette): flags((u16)TextureTiles::NONE), type((u16)TextureType::BGR5), stride(2U) {
	TTLP &ttlp = palette.at<0>();
	width = ttlp.c_colors;
	dataSize = ttlp.dataSize;
	size = dataSize / stride;
	height = size / width;
	data = palette.get<0>().ptr;
}

Texture2D::Texture2D(NCGR &tilemap): flags((u16)TextureTiles::TILED8), stride(1U) {

	RAHC &rahc = tilemap.at<0>();
	bool fourBit = rahc.tileDepth == 3;
	type = fourBit ? (u16)TextureType::R4 : (u16)TextureType::INTEGER;

	dataSize = rahc.tileDataSize;
	size = dataSize * (fourBit ? 2U : 1U);
	
	if (rahc.tileWidth == u16_MAX || rahc.tileHeight == u16_MAX) {
		width = (u32) sqrt(size);
		height = width;
		printf("Texture2D Warning; Non-Sized Texture2D found with NCGR. Resizing to %u %u, please call 'changeDimensions' if this size is incorrect\n", width, height);
	} else {
		width = rahc.tileWidth * 8U;
		height = rahc.tileHeight * 8U;
	}

	data = tilemap.get<0>().ptr;
}

Texture2D::Texture2D(NSCR &map): flags((u16)TextureTiles::NONE), stride(2U), type((u16)TextureType::INTEGER) {

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
	return tex;
}

Texture2D Texture2D::read(std::string file) {
	int x, y, channels;
	u8 *ptr = (u8*) stbi_load(file.c_str(), &x, &y, &channels, 4);
	return { ptr, (u16)x, (u16)y, 4U, TextureType::RGBA8, TextureTiles::NONE };
}

void Texture2D::write(std::string file) {

	if (type != (u16)TextureType::RGBA8 || flags != (u16)TextureTiles::NONE) 
		EXCEPTION("Texture2D Couldn't write image; please convert to RGBA8 first");

	if (file.size() < 4 || std::string(file.end() - 4, file.end()) != ".png")
		EXCEPTION("Texture2D Couldn't write image; it only supports .png");

	if (!stbi_write_png(file.c_str(), (int)width, (int)height, 4, data, 4 * (int)width))
		EXCEPTION("Texture2D Couldn't write image");

}

void Texture2D::dealloc() {
	if (data != nullptr) {
		Buffer b = { getDataSize(), data };
		b.dealloc();
		data = nullptr;
	}
}

TextureType Texture2D::getType() { return (TextureType)type; }

u16 Texture2D::getWidth() { return width; }
u16 Texture2D::getHeight() { return height; }
u32 Texture2D::getSize() { return size; }
u32 Texture2D::getTiles() { return (u32) flags; }
u32 Texture2D::getDataSize() { return width * height * stride; }

u32 Texture2D::changeDimensions(u16 w, u16 h) {

	u32 siz = (u32)w * (u32)h;

	if (size != siz) {
		printf("Texture2D Couldn't change dimensions; sizes didn't match\n");
		return 0;
	}

	if (getTiles() != 0 && !(w % getTiles() == 0 && h % getTiles() == 0)) {
		printf("Texture2D Couldn't change dimensions; size(s) do(es)n't match the tiling\n");
		return 1;
	}

	width = w;
	height = h;
	return 2;
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

	if (fourBit && index % 2U == 0U)
		val &= 0xFU;
	else if (fourBit)
		val >>= 4U;

	return val;
}

u32 fromBGR5(u32 val) {
	return (u32)(val / 31.f * 255);
}

u32 Texture2D::read(u16 i, u16 j) {

	u32 pix = fetch(i, j);

	if (type == (u16)TextureType::BGR5){

		u32 b = fromBGR5(pix & 0x1FU);
		u32 g = fromBGR5((pix & 0x3E0U) >> 5U);
		u32 r = fromBGR5((pix & 0x7C00U) >> 10U);

		pix = (0xFFU << 24U) | (r << 16U) | (g << 8U) | b;
	}

	return pix;
}

bool Texture2D::store(u16 i, u16 j, u32 k) {

	if (size == 0 || data == nullptr || stride > 4U)
		return false;

	i %= width;
	j %= height;

	u32 index = getIndex(i, j);

	bool fourBit = type == (u16)TextureType::R4;

	u8 *ptr = data + index * stride / (fourBit ? 2U : 1U);

	if (fourBit && index % 2U == 0U)
		*ptr = (*ptr & 0xF0U) | (k & 0xFU);
	else if (fourBit)
		*ptr = (*ptr & 0xFU) | ((k & 0xFU) << 4U);
	else
		memcpy(ptr, &k, stride);

	return true;
}

u32 toBGR5(u32 val) {
	return (u32)(val / 255.f * 31);
}

bool Texture2D::write(u16 i, u16 j, u32 k) {

	
	if (type == (u16)TextureType::BGR5) {

		u32 r = toBGR5(k & 0xFFU);
		u32 g = toBGR5((k & 0xFF00U) >> 8U);
		u32 b = toBGR5((k & 0xFF0000U) >> 16U);

		k = b | (g << 5U) | (r << 10U);
	}

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