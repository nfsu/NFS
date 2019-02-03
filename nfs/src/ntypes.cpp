#include "ntypes.h"
using namespace nfs;

NDSBanner *NDSBanner::get(NDS *nds) {
	return (NDSBanner*)((u8*)nds + nds->iconOffset);
}

std::vector<NDSTitle> NDSBanner::getTitles() {

	std::vector<std::wstring> strings;
	strings.reserve(6);

	for(u32 i = 0; i < 6; ++i)
		if (titles[i][0] != u16_MAX && titles[i][0] != 0) {

			std::wstring name = (wchar_t*)titles[i];
			bool unique = true;

			for(u32 j = 0, k = (u32) strings.size(); j < k; ++j)
				if (name == strings[j]) {
					unique = false;
					break;
				}

			if(unique)
				strings.push_back(name);
		}

	std::vector<NDSTitle> titles(strings.size());

	for (u32 i = 0, j = (u32)strings.size(); i < j; ++i) {

		std::wstring &str = strings[i];
		auto titleEnd = std::find(str.begin(), str.end(), L'\n');
		auto subtitleEnd = std::find(titleEnd + 1, str.end(), L'\n');

		std::wstring stitle = str.substr(0, titleEnd - str.begin());
		std::wstring ssubtitle = str.substr(titleEnd - str.begin() + 1, subtitleEnd - titleEnd - 1);
		std::wstring sauthor;

		if (subtitleEnd == str.end()) {
			sauthor = ssubtitle;
			ssubtitle = L"";
		} else {
			sauthor = str.substr(subtitleEnd - str.begin() + 1, str.end() - subtitleEnd);
		}

		titles[i] = NDSTitle(stitle, ssubtitle, sauthor);
	}

	return titles;
}