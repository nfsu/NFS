#include "ntypes.hpp"
using namespace nfs;

NDSBanner *NDSBanner::get(NDS *nds) {
	return (NDSBanner*)((u8*)nds + nds->iconOffset);
}

WString NDSBanner::getTitle(NDSTitleLanguage lang) {
	return WString((c16*) titles[usz(lang)]);
}