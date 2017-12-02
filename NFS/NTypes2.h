#pragma once
#include "Types.h"
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <exception>
#include "API/LM4000_TypeList/TypeStruct.h"

#ifdef __X64__
#define GenericSection_begin 16
#else
#define GenericSection_begin 8
#endif

namespace nfs {

	class FileSystem;

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

	//NDS file format
	struct NDS {
		Buffer data;
		char title[12];
		char gameCode[4];
		char makerCode[2];
		u8 unitCode;
		u8 encryptionSeed;
		u8 capacity;
		u8 reserved[7];
		u8 unknown[2];			//Used by DSi titles
		u8 version;
		u8 flags;				//Flags such as auto start (Bit2)
		u32 arm9_offset;		//Rom offset for ARM9
		u32 arm9_entry;			//Entry address for ARM9
		u32 arm9_load;			//Load address for ARM9
		u32 arm9_size;			//Size for ARM9
		u32 arm7_offset;		//Rom offset for ARM7
		u32 arm7_entry;			//Entry address for ARM7
		u32 arm7_load;			//Load address for ARM7
		u32 arm7_size;			//Size for ARM7
		u32 ftable_off;			//File name table offset
		u32 ftable_len;			//File name table length
		u32 falloc_off;			//File allocation table offset
		u32 falloc_len;			//File allocation table length
		u32 arm9_ooff;			//Overlay offset for ARM9
		u32 arm9_olen;			//Overlay length for ARM9
		u32 arm7_ooff;			//Overlay offset for ARM7
		u32 arm7_olen;			//Overlay length for ARM7
		u32 cardControl;		//Normal card control register settings
		u32 sCardControl;		//Safe card control register settings
		u32 iconOffset;
		u16 sAC;				//Secure area checksum
		u16 sALT;				//Secure area loading timeout
		u32 arm9_ALLRA;			//Auto load list RAM address
		u32 arm7_ALLRA;			//Auto load list RAM address
		u64 sAD;				//Secure area disable
		u32 romSize;
		u32 romHeaderSize;
		u8 reserved1[56];
		u8 nLogo[156];
		u16 nLC;				//Logo Checksum
		u16 nHC;				//Header Checksum
		u32 dRomOff;			//Debug rom offset
		u32 dRomSize;			//Debug rom size
		u32 reserved2;
		u8 reserved3[144];
	};

	///Custom types

	//Buffer info
	struct NBIS : GenericSection { 
		std::string name;
	};

	//Buffer unknown object; used for storing buffers of unknown types
	typedef GenericResource<NBIS> NBUO;

	//Physical archive

	class NArchieve {

	public:

		NArchieve(std::vector<GenericResourceBase*> _resources, Buffer _buf);
		NArchieve();
		~NArchieve();

		NArchieve(const NArchieve &other);

		NArchieve &operator=(const NArchieve &other);

		template<class T>
		T &operator[](u32 i) const {
			return get<T>(i);
		}

		template<class T>
		T &get(u32 i) const {
			static_assert(std::is_base_of<GenericResourceBase, T>::value, "operator<T>[] where T should be instanceof GenericResourceBase");

			if (i >= resources.size())
				throw(std::exception("Out of bounds"));

			T *t = NType::castResource<T>(resources[i]);

			if (t == nullptr)
				throw(std::exception("Couldn't convert resource"));

			return *t;
		}

		u32 getType(u32 i) const;
		std::string getTypeName(u32 i) const;
		u32 size() const;
		u32 bufferSize() const;

	protected:

		void copy(const NArchieve &other);

	private:

		Buffer buf;
		std::vector<GenericResourceBase*> resources;
	};

	///LM4000 archieve magic

	//TypeList with all the archive types
	typedef lag::TypeList<NARC, NCSR, NCGR, NCLR, NBUO> ArchiveTypes;

	//function type
	template<typename ... Args>
	using archive_func = void(Args...);

	//map return type
	template<typename ... Args>
	using archive_fp_map = std::unordered_map<u32, archive_func<Args...>*>;

	//archive functions
	template<typename T, template<typename> typename F, typename ... Args>
	void genericFunc(Args ... args)
	{
		F<T>{}(args...);
	}

	template<typename ... Types>
	struct MakeArchiveFpMapStruct;

	//make function pointer map
	template<template<typename> typename F, typename ... Args, typename ... Types>
	archive_fp_map<Args...> makeArchiveFpMap(lag::TypeList<Types...> tl);

	//make static instance of function pointer map and return a reference to it
	template<template<typename> typename F, typename ... Args, typename ... Types>
	archive_fp_map<Args...> &getArchiveFpMap(lag::TypeList<Types...> tl)
	{
		const static auto fpMap = makeArchiveFpMap<F, Args...>(tl);
		return (archive_fp_map<Args...> &)fpMap;
	}

	//run a function from the function pointer map
	template<template<typename> typename F, typename ... Args, typename ... Types>
	void runArchiveFunction(u32 magicNum, lag::TypeList<Types...> tl, Args ... args)
	{
		auto &fpMap = getArchiveFpMap<F, Args...>(tl);
		auto &f = fpMap[magicNum];

		if (f != nullptr)
			f(args...);
		else {
			std::string where = "Function not found (" + std::string(typeid(F).name()) + "; " + (char*)&magicNum + ")";
			throw(std::exception(where.c_str()));
		}
	}

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

		template<> constexpr static u32 get<NBUO> = 0x0;

	};

	struct SectionLength {

		template<typename T> constexpr static u32 get = 0;
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

		template<typename T>
		static bool readGenericStruct(T *wh, Buffer from) {

			static_assert(std::is_base_of<GenericSection, T>::value, "readGenericStruct<T> where T is instanceof GenericResourceBase");

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

				std::string errorstr = "Couldn't read " + std::string(typeid(T).name()) + " \"" + error + "\"";

				throw(std::exception(errorstr.c_str()));
				return false;
			}

			return true;
		}

		template<typename T>
		struct NFactory {
			void operator()(void *first, Buffer buf) {
				NType::readGenericResource((T*)first, buf);
			}
		};

		template<>
		struct NFactory<NBUO> {
			void operator()(void *first, Buffer buf) {
				NBUO &buo = *(NBUO*)first;
				buo.contents.front.data = buf;
			}
		};

		template<typename T>
		struct IsValidType {
			void operator()(bool *b) {
				*b = true;
			}
		};

		template<>
		struct IsValidType<NBUO> {
			void operator()(bool *b) {
				*b = false;
			}
		};

		template<typename T>
		struct GenericResourceSize {
			void operator()(u32 *result) {
				*result += sizeof(T);
			}
		};

		template<class T, class T2> static bool convert(T source, T2 *target) { return false; }

		template<> static bool convert(NARC source, NArchieve *archieve);
		template<> static bool convert(NCLR source, Texture2D *tex);
		template<> static bool convert(NCGR source, Texture2D *tex);
		template<> static bool convert(NCSR source, Texture2D *tex);
		template<> static bool convert(NDS nds, FileSystem *fs);

		template<typename T>
		static T *castResource(GenericResourceBase *wh) {
			if (wh->header.magicNumber == MagicNumber::get<T>)
				return (T*)wh;
			return nullptr;
		}

		static NDS readNDS(Buffer buf);
	};

	//moved functions from archive function map init
	template<typename First, typename ... Types>
	struct MakeArchiveFpMapStruct<First, Types...>
	{
		template<template<typename> typename F, typename ... Args>
		static void insert(archive_fp_map<Args...> &map)
		{
			archive_func<Args...> *push = &genericFunc<First, F, Args...>;
			map[MagicNumber::get<First>] = push;
			MakeArchiveFpMapStruct<Types...>::insert<F>(map);
		}
	};

	template<typename First>
	struct MakeArchiveFpMapStruct<First>
	{
		template<template<typename> typename F, typename ... Args>
		static void insert(archive_fp_map<Args...> &map)
		{
			archive_func<Args...> *push = &genericFunc<First, F, Args...>;
			map[MagicNumber::get<First>] = push;
		}
	};

	template<template<typename> typename F, typename ...Args, typename ...Types>
	archive_fp_map<Args...> makeArchiveFpMap(lag::TypeList<Types...> tl)
	{
		archive_fp_map<Args...> ret = archive_fp_map<Args...>();
		MakeArchiveFpMapStruct<Types...>::insert<F, Args...>(ret);
		return ret;
	}


	template<> static bool NType::convert(NARC source, NArchieve *archieve) {

		BTAF &btaf = source.contents.front;
		u32 files = btaf.files;

		if (btaf.data.size != btaf.files * 8 || files == 0) {
			printf("Couldn't convert NARC to Archieve; invalid file info\n");
			return false;
		}

		u32 bufferSize = 0;

		for (u32 i = 0; i < files; ++i) {

			u32 off = getUInt(offset(btaf.data, i * 8));
			u32 size = getUInt(offset(btaf.data, i * 8 + 4)) - off;
			u8 *data = source.contents.back.back.front.data.data + off;

			u32 magicNumber = *(u32*)data;

			u32 offInBuffer = bufferSize;

			try {
				runArchiveFunction<GenericResourceSize>(magicNumber, ArchiveTypes(), &bufferSize);
			}
			catch (std::exception e) {}

			if (offInBuffer == bufferSize)
				bufferSize += sizeof(NBUO);
		}

		u32 currOff = 0;

		u32 someOtherInt = *(u32*)(source.contents.back.front.data.data + 4);

		Buffer buf = newBuffer1(bufferSize);
		std::vector<GenericResourceBase*> resources(files);

		for (u32 i = 0; i < files; ++i) {

			u32 off = getUInt(offset(btaf.data, i * 8));
			u32 size = getUInt(offset(btaf.data, i * 8 + 4)) - off;
			u8 *data = source.contents.back.back.front.data.data + off;

			u32 magicNumber = *(u32*)data, oMagicNumber = magicNumber;

			u32 offInBuffer = currOff;
			
			try{
				runArchiveFunction<GenericResourceSize>(magicNumber, ArchiveTypes(), &currOff);
			}
			catch (std::exception e) {}

			u8 *loc = buf.data + offInBuffer;
			Buffer b = { data, size };

			if (currOff == offInBuffer) {
				currOff += sizeof(NBUO);
				magicNumber = 0;
			}

			try {
				runArchiveFunction<NFactory>(magicNumber, ArchiveTypes(), (void*)loc, b);
			}
			catch (std::exception e) {}

			resources[i] = (GenericResourceBase*)loc;
		}

		*archieve = NArchieve(resources, buf);

		return true;
	}

	template<> bool NType::convert(NCLR source, Texture2D *tex) {
		tex->width = source.contents.front.c_colors;
		tex->size = source.contents.front.dataSize;
		tex->stride = 2;
		tex->tt = BGR5;
		tex->height = tex->size / tex->stride / tex->width;
		tex->data = source.contents.front.data.data;
		return true;
	}


	template<> bool NType::convert(NCGR source, Texture2D *tex) {
		bool fourBit = source.contents.front.tileDepth == BD_FOUR;

		tex->width = source.contents.front.tileWidth * 8;
		tex->height = source.contents.front.tileHeight * 8;
		tex->size = tex->width * tex->height / (fourBit ? 2 : 1);
		tex->tt = fourBit ? TILED8_B4 : TILED8;
		tex->stride = 1;
		tex->data = source.contents.front.data.data;
		return true;
	}


	template<> bool NType::convert(NCSR source, Texture2D *tex) {
		tex->width = source.contents.front.screenWidth / 8;
		tex->height = source.contents.front.screenHeight / 8;
		tex->size = tex->width * tex->height * 2;
		tex->tt = NORMAL;
		tex->stride = 2;
		tex->data = source.contents.front.data.data;
		return true;
	}
}