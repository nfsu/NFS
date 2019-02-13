#include "ntypes.h"
using namespace nfs;

NDSBanner *NDSBanner::get(NDS *nds) {
	return (NDSBanner*)((u8*)nds + nds->iconOffset);
}

std::wstring NDSBanner::getTitle(NDSTitleLanguage lang) {
	return std::wstring((wchar_t*)titles[(u32)lang]);
}