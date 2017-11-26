#include "NHelper.h"
using namespace nfs;

PaletteTexture2D NHelper::readNCGR(Buffer buf, u32 paletteOff, u32 textureOff) {

	NCLR nclr;
	NType::readGenericResource(&nclr, offset(buf, paletteOff));

	NCGR ncgr;
	NType::readGenericResource(&ncgr, offset(buf, textureOff));

	Texture2D palette;
	NType::convert(nclr, &palette);

	Texture2D tex;
	NType::convert(ncgr, &tex);

	return { palette, tex };
}

TiledTexture2D NHelper::readNCSR(Buffer buf, u32 paletteOff, u32 tilemapOff, u32 mapOff) {

	PaletteTexture2D pt2d = readNCGR(buf, paletteOff, tilemapOff);

	NCSR ncsr;
	NType::readGenericResource(&ncsr, offset(buf, mapOff));

	Texture2D map;
	NType::convert(ncsr, &map);

	return { pt2d.palette, pt2d.tilemap, map };
}

bool NHelper::writeNCGR(Buffer buf, u32 paletteOff, u32 tilemapOff, std::string path) {
	PaletteTexture2D pt2d = NHelper::readNCGR(buf, paletteOff, tilemapOff);

	Texture2D tex2 = convertPT2D(pt2d);
	bool res = writeTexture(tex2, path);
	deleteTexture(&tex2);

	return res;
}

bool NHelper::writeNCSR(Buffer buf, u32 paletteOff, u32 tilemapOff, u32 mapOff, std::string path) {
	TiledTexture2D tt2d = NHelper::readNCSR(buf, paletteOff, tilemapOff, mapOff);

	Texture2D tex2 = convertTT2D(tt2d);
	bool res = writeTexture(tex2, path);
	deleteTexture(&tex2);

	return res;
}