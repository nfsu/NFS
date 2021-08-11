#include "resource_helper.hpp"
using namespace nfs;

u32 ResourceHelper::getType(const u8 *type, u32 size) {
	return getType_inter(ResourceTypes{}, type, size);
}

size_t ResourceHelper::getSize(u32 typeId) {
	return getSize_inter(ResourceTypes{}, typeId);
}

u32 ResourceHelper::getMagicNum(u32 typeId) {
	return getMagicNumber_inter(ResourceTypes{}, typeId);
}

ResourceInfo ResourceHelper::read(u8 *rom, usz size, u8 *res) {

	ResourceInfo info;
	info.type = getType(rom, size);
	info.size = getSize(info.type);
	info.magicNumber = getMagicNum(info.type);

	fill_inter(ResourceTypes{}, info.type, res, rom, size);

	return info;
}

String ResourceHelper::getName(u8 *loc, usz len, u32 magicNumber, bool flip) {

	if (magicNumber == NBUO_num) {

		u32 num;

		switch (len) {

			case 0: 
				magicNumber = 0;
				break;

			case 1:
				magicNumber = *(const u8*)loc;
				break;

			case 2:
				magicNumber = *(const u16*)loc;
				break;

			case 3: 
				magicNumber = (*(const u16*)loc) + (u32(*(const u8*)(loc + 2)) << 16);
				break;

			default:
				magicNumber = *(const u32*)loc;
				break;
		}
	}

	c8 dat[5] = { 
		(c8)((magicNumber >> 24) & 0xFF), 
		(c8)((magicNumber >> 16) & 0xFF), 
		(c8)((magicNumber >> 8) & 0xFF), 
		(c8)(magicNumber & 0xFF), '\0' 
	};

	for (u32 i = 0; i < 4; ++i) {

		char &c = dat[i];

		c = flip ? c8((magicNumber >> (24 - i * 8)) & 0xFF) : c8((magicNumber >> (i * 8)) & 0xFF);

		if (c >= 'a' && c <= 'z')
			c = c - 'a' + 'A';

		if (c >= 'A' && c <= 'Z') continue;
		if (c >= '0' && c <= '9') continue;

		c = '?';
	}

	return String(dat);

}