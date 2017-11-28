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

	class NArchieve {

	public:

		NArchieve(std::vector<GenericResourceBase*> _resources, Buffer _buf) : resources(_resources), buf(_buf) {}
		NArchieve() {}
		~NArchieve() { deleteBuffer(&buf); }

		NArchieve(const NArchieve &other) {
			copy(other);
		}

		NArchieve &operator=(const NArchieve &other) {
			copy(other);
			return *this;
		}

		template<class T = typename std::enable_if<std::is_base_of<GenericResourceBase, T>::type>::value>
		T &operator[](u32 i) const {
			if (i >= resources.size())
				throw(std::exception("Out of bounds"));

			T *t = NType::castResource<T>(resources[i]);

			if (t == nullptr)
				throw(std::exception("Couldn't convert resource"));

			return *t;
		}

		u32 getType(u32 i) const {
			if (i >= resources.size())
				throw(std::exception("Out of bounds"));

			return resources[i]->header.magicNumber;
		}

		std::string getTypeName(u32 i) const {
			u32 t = getType(i);
			std::string typeName = std::string((char*)&t, 4);
			std::reverse(typeName.begin(), typeName.end());
			return typeName;
		}

		u32 size() const { return resources.size(); }
		u32 bufferSize() const { return buf.size; }

	protected:

		void copy(const NArchieve &other) {

			if (other.buf.data != NULL)
				buf = newBuffer3(other.buf.data, other.buf.size);
			else
				buf = { NULL, 0 };

			resources = other.resources;

			for (u32 i = 0; i < other.size(); ++i)
				resources[i] = (GenericResourceBase*)(((u8*)other.resources[i]) - other.buf.data + buf.data);
		}

	private:

		Buffer buf;
		std::vector<GenericResourceBase*> resources;
	};

	///LM4000 archieve magic

	//TypeList with all the archive types
	typedef lag::TypeList<NARC, NCSR, NCGR, NCLR> ArchiveTypes;

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
		fpMap[magicNum](args...);
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

		template<typename T>
		struct NFactory {
			void operator()(void *first, Buffer buf) {
				NType::readGenericResource((T*)first, buf);
			}
		};

		template<typename T>
		struct GenericResourceSize {
			void operator()(u32 *result) {
				*result += sizeof(T);
			}
		};

		template<class T, class T2> static bool convert(T source, T2 *target) { return false; }

		template<> static bool convert(NARC source, NArchieve *archieve) {

			BTAF &btaf = source.contents.front;
			u32 files = btaf.files;

			if (btaf.data.size != btaf.files * 8 || files == 0) {
				printf("Couldn't convert NARC to Archieve; invalid file info\n");
				return false;
			}

			u32 bufferSize = 0, objects = 0;

			for (u32 i = 0; i < files; ++i) {

				u32 off = getUInt(offset(btaf.data, i * 8));
				u32 size = getUInt(offset(btaf.data, i * 8 + 4)) - off;
				u8 *data = source.contents.back.back.front.data.data + off;

				u32 magicNumber = *(u32*)data;

				u32 offInBuffer = bufferSize;
				runArchiveFunction<GenericResourceSize>(magicNumber, ArchiveTypes(), &bufferSize);

				if (offInBuffer != bufferSize)
					++objects;
			}

			u32 currOff = 0, objOff = 0;

			Buffer buf = newBuffer1(bufferSize);
			std::vector<GenericResourceBase*> resources(objects);

			for (u32 i = 0; i < files; ++i) {

				u32 off = getUInt(offset(btaf.data, i * 8));
				u32 size = getUInt(offset(btaf.data, i * 8 + 4)) - off;
				u8 *data = source.contents.back.back.front.data.data + off;

				u32 magicNumber = *(u32*)data;

				u32 offInBuffer = currOff;
				runArchiveFunction<GenericResourceSize>(magicNumber, ArchiveTypes(), &currOff);

				if (currOff != offInBuffer) {
					u8 *loc = buf.data + offInBuffer;
					Buffer b = { data, size };
					runArchiveFunction<NFactory>(magicNumber, ArchiveTypes(), (void*)loc, b);
					resources[objOff] = (GenericResourceBase*)loc;
					++objOff;
				}
			}

			*archieve = NArchieve(resources, buf);

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


		template<> static bool convert(NCSR source, Texture2D *tex) {
			tex->width = source.contents.front.screenWidth / 8;
			tex->height = source.contents.front.screenHeight / 8;
			tex->size = tex->width * tex->height * 2;
			tex->tt = NORMAL;
			tex->stride = 2;
			tex->data = source.contents.front.data.data;
			return true;
		}

		template<typename T>
		static T *castResource(GenericResourceBase *wh) {
			if (wh->header.magicNumber == MagicNumber::get<T>)
				return (T*)wh;
			return nullptr;
		}

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
}