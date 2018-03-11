#include "resourcehelper.h"
using namespace nfs;

u32 ResourceHelper::getType(const u8 *type) {
	return getType_inter(ResourceTypes{}, type);
}

size_t ResourceHelper::getSize(u32 typeId) {
	return getSize_inter(ResourceTypes{}, typeId);
}

u32 ResourceHelper::getMagicNum(u32 typeId) {
	return getMagicNumber_inter(ResourceTypes{}, typeId);
}

ResourceInfo ResourceHelper::read(u8 *rom, u32 size, u8 *res) {

	ResourceInfo info;
	info.type = getType(rom);
	info.size = getSize(info.type);
	info.magicNumber = getMagicNum(info.type);

	fill_inter(ResourceTypes{}, info.type, res, rom, size);

	return info;
}

std::string ResourceHelper::getName(u8 *loc, u32 magicNumber) {

	if (magicNumber == NBUO_num) {
		magicNumber = *(u32*)loc;
	}

	char dat[5] = { (char)((magicNumber & 0xFF000000U) >> 24U), (char)((magicNumber & 0xFF0000U) >> 16U), (char)((magicNumber & 0xFF00U) >> 8U), (char)(magicNumber & 0xFFU), '\0' };

	for (u32 i = 0; i < 4; ++i) {

		char &c = dat[i];

		if (c >= 'a' && c <= 'z')
			c = c - 'a' + 'A';

		if (c >= 'A' && c <= 'Z') continue;
		if (c >= '0' && c <= '9') continue;

		c = '?';
	}

	return std::string(dat);

}