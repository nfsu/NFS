#pragma once
#include "Types.h"
#include <type_traits>
#include <typeinfo>
#include "API/LM4000_TypeList/TypeStruct.h"

#ifdef __X64__
#define GenericSection_begin 16
#else
#define GenericSection_begin 8
#endif

namespace nfs {

	//A generic header used for sections
	struct GenericSection {
		Buffer data;
		u32 magicNumber;					//MagicNumber; TTLP, PMCP, etc.
		u32 size;							//Size of section; including contents
	};

	struct GenericHeader {
		u32 magicNumber;					//MagicNumber; NCLR, NCGR, NSCR, etc.
		u32 c_constant;
		u32 size;							//Size of the header; including contents
		u16 c_headerSize;
		u16 sections;						//Number of sections
	};

	struct GenericResourceBase {
		GenericHeader header;
	};

	template<typename ...args> struct GenericResource : GenericResourceBase {
		lag::TypeStruct<args...> contents;	//Contents of resource
	};

	//Palette data (data is stored as BGR555)
	struct TTLP : GenericSection {
		u32 bitDepth;						//3 = 4 bits, 4 = 8 bits
		u32 c_padding;						//0x00000000
		u32 dataSize;						//size of palette data in bytes; if(size > 0x200) size = 0x200 - size;
		u32 c_colors;						//0x00000010
	};

	//Palette count map?
	struct PMCP : GenericSection {
		u16 count;							//Count of palettes in file
		u32 c_constant;						//0xEFBE08
		u16 c_padding;						//0x0000
	};

	//Color resource
	typedef GenericResource<TTLP, PMCP> NCLR;

	//Character Data
	struct RAHC : GenericSection {
		u16 tileHeight;
		u16 tileWidth;
		u32 tileDepth;					//1 << (tileDepth - 1) = bit depth
		u64 c_padding;					//0x0000000000000000
		u32 tileDataSize;				//tileDataSize / 1024 = tileCount; tileDataSize * (2 - (tileDepth - 3)) = pixels
		u32 c_padding0;					//0x00000018
	};

	struct SOPC : GenericSection {
		u32 c_padding;				//0x00000000
		u16 tileWidth;				//= tileCount in RAHC
		u16 tileHeight;				//= RAHC tileCount
	};

	//Graphics resource
	typedef GenericResource<RAHC, SOPC> NCGR;

	struct NRCS : GenericSection {
		u16 screenWidth;				//Width of screen (pixels)
		u16 screenHeight;				//Height of screen (pixels)
		u32 c_padding;					//unknown
		u32 screenDataSize;				//Size of screen data buffer
	};

	//Screen resource
	typedef GenericResource<NRCS> NCSR;

	//File allocation table
	struct BTAF : GenericSection {
		u32 files;						//Count of files in archive
	};

	//File name table
	struct BTNF : GenericSection {};

	//File image
	struct GMIF : GenericSection { };

	//Archive file
	typedef GenericResource<BTAF, BTNF, GMIF> NARC;

	//Physical archive
	typedef Archieve<GenericResourceBase*> NArchieve;

	///MagicNumbers


	struct MagicNumber {

		template<typename T> constexpr static u32 get = 0x0;
		template<> constexpr static u32 get<TTLP> = 0x504C5454;
		template<> constexpr static u32 get<PMCP> = 0x50434D50;
		template<> constexpr static u32 get<NCLR> = 0x4E434C52;
		template<> constexpr static u32 get<RAHC> = 0x43484152;
		template<> constexpr static u32 get<SOPC> = 0x43504F53;
		template<> constexpr static u32 get<NCGR> = 0x4E434752;
		template<> constexpr static u32 get<NRCS> = 0x5343524E;
		template<> constexpr static u32 get<NCSR> = 0x4E534352;
		template<> constexpr static u32 get<GMIF> = 0x46494D47;
		template<> constexpr static u32 get<BTAF> = 0x46415442;
		template<> constexpr static u32 get<NARC> = 0x4352414E;
		template<> constexpr static u32 get<BTNF> = 0x464E5442;

	};

	struct SectionLength {

		template<typename T> constexpr static u32 get = 0x0;
		template<> constexpr static u32 get<TTLP> = 24;
		template<> constexpr static u32 get<PMCP> = 16;
		template<> constexpr static u32 get<RAHC> = 32;
		template<> constexpr static u32 get<SOPC> = 16;
		template<> constexpr static u32 get<NRCS> = 20;
		template<> constexpr static u32 get<GMIF> = 8;
		template<> constexpr static u32 get<BTAF> = 12;
		template<> constexpr static u32 get<BTNF> = 8;
	};

	///Reading NTypes

	struct NType {

		template<typename T = typename std::enable_if<std::is_base_of<GenericResourceBase, T>::value>::type>
		static bool readGenericStruct(T *wh, Buffer from) {

			u32 size = SectionLength::get<T>;

			const char *error = nullptr;
			GenericSection *gs = nullptr;
			u32 bufferSize = 0;

			if (size > from.size) {
				error = "Invalid buffer size";
				goto end;
			}

			gs = (GenericSection*)wh;
			memcpy((u8*)gs + GenericSection_begin, from.data, size);

			bufferSize = gs->size - size;
			gs->data.size = bufferSize;
			gs->data.data = from.data + size;

			if (gs->size > from.size) {
				error = "Invalid buffer size";
				goto end;
			}

			if (gs->magicNumber != MagicNumber::get<T>)
				error = "Invalid magicNumber";

		end:

			if (error != nullptr) {
				memset(wh, 0, sizeof(T));
				printf("Couldn't read %s; %s\n", typeid(T).name(), error);
				return false;
			}

			return true;
		}

		template<typename GR, typename ... Types>
		struct SectionLoop;

		template<typename GR, typename First, typename ... Types>
		struct SectionLoop<GR, First, Types...>
		{
			static bool read(GR *ptr, u32 &off, u32 &offsetId, Buffer b, u32 maxSections)
			{
				if (!SectionLoop<GR, First>::read(ptr, off, offsetId, b, maxSections))
					return false;

				return SectionLoop<GR, Types...>::read(ptr, off, offsetId, b, maxSections);
			}
		};

		template<typename GR, typename T>
		struct SectionLoop<GR, T>
		{
			static bool read(GR *ptr, u32 &off, u32 &offsetId, Buffer b, u32 maxSections)
			{
				if (maxSections == offsetId)
					return true;

				T *gs = &ptr->contents.getAt<T>(offsetId);

				if (!readGenericStruct<T>(gs, offset(b, off))) {
					printf("Couldn't read section %s\n", typeid(T).name());
					return false;
				}

				off += gs->size;
				++offsetId;
				return true;
			}
		};

		template<typename ...args>
		static bool readGenericResource(GenericResource<args...> *wh, Buffer from) {

			using T = GenericResource<args...>;

			memset((u8*)wh, 0, sizeof(T));

			u32 size = sizeof(GenericHeader);

			const char *error = nullptr;
			GenericHeader *gh = (GenericHeader*)wh;

			if (size > from.size) {
				error = "Invalid buffer size";
				goto end;
			}

			memcpy(wh, from.data, size);

			if (gh->magicNumber != MagicNumber::get<T>) {
				error = "Invalid magicNumber";
				goto end;
			}

			u32 off = 0, offId = 0;

			if (!SectionLoop<GenericResource<args...>, args...>::read(wh, off, offId, offset(from, size), gh->sections)) {
				error = "Invalid section";
				goto end;
			}

		end:

			if (error != nullptr) {
				memset(wh, 0, sizeof(T));
				printf("Couldn't read %s; %s\n", typeid(T).name(), error);
				return false;
			}

			return true;
		}

		template<class T, class T2> static bool convert(T source, T2 *target) { return false; }

		template<> static bool convert(NARC source, NArchieve *archieve) {

			BTAF &btaf = source.contents.front;
			std::vector<ArchieveObject<GenericResourceBase*>> arcobj(btaf.files);

			if (btaf.data.size != btaf.files * 8) {
				printf("Couldn't convert NARC to Archieve; invalid file info\n");
				return false;
			}

			for (u32 i = 0; i < arcobj.size(); ++i) {
				ArchieveObject<GenericResourceBase*> &elem = arcobj[i];

				elem.id = i;
				elem.offset = getUInt(offset(btaf.data, i * 8));
				elem.contents.size = getUInt(offset(btaf.data, i * 8 + 4)) - elem.offset;
				elem.contents.data = source.contents.back.back.front.data.data + elem.offset;
				elem.type = std::string((char*)elem.contents.data, 4);

			}

			*archieve = NArchieve(arcobj, source.contents.back.back.front.size);

			return true;
		}

		template<> static bool convert(NCLR source, Texture2D *tex) {
			tex->width = source.contents.front.c_colors;
			tex->size = source.contents.front.dataSize;
			tex->stride = 2;
			tex->tt = BGR5;
			tex->height = tex->size / tex->stride / tex->width;
			tex->data = source.contents.front.data.data;
			return true;
		}


		template<> static bool convert(NCGR source, Texture2D *tex) {
			bool fourBit = source.contents.front.tileDepth == BD_FOUR;
			
			tex->width = source.contents.front.tileWidth * 8;
			tex->height = source.contents.front.tileHeight * 8;
			tex->size = tex->width * tex->height / (fourBit ? 2 : 1);
			tex->tt = fourBit ? TILED8_B4 : TILED8;
			tex->stride = 1;
			tex->data = source.contents.front.data.data;
			return true;
		}

		template<typename T, typename ...args>
		static T *castResource(GenericResource<args...> *wh) {
			if (wh->header.magicNumber == MagicNumber::get<T>)
				return (T*)wh;
			return nullptr;
		}

	};
}