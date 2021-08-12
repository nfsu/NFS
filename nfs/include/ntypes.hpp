#pragma once
#include "generic_resource.hpp"
#include "compression_helper.hpp"

namespace nfs {

	//NCLR (Palette file)

	struct TTLP : GenericSection {												//palette data
		u32 bitDepth;						//3 = 4 bits, 4 = 8 bits
		u32 c_padding;						//0x00000000
		u32 dataSize;						//size of palette data in bytes; if(size > 0x200) size = 0x200 - size;
		u32 c_colors;						//0x00000010
	};

	struct PMCP : GenericSection {												//Palette count map
		u16 count;							//Count of palettes in file
		u32 c_constant;						//0xEFBE08
		u16 c_padding;						//0x0000
	};

	typedef GenericResource<0x4E434C52, false, TTLP, PMCP> NCLR;				//Palette ("Color") resource

	//NCGR (Tile file / image file)

	//The contents of RAHC can be encrypted;
	//This means that the image needs to be XORed with a magic texture
	//u32 seed = texture[end()];
	//for(i32 i = end(); i >= begin(); --i) { magic[i] = seed; seed = CompressionHelper::generateRandom(seed); }
	struct RAHC : GenericSection {												//Character data
		u16 tileHeight;
		u16 tileWidth;
		u16 tileDepth;						//1 << (tileDepth - 1) = bit depth
		u16 sizeHint0;						//0x0A or 0x00
		u16 sizeHint1;						//0x10 or 0x00 might be a size hint
		u16 sizeHint2;						//0x10 or 0x00 might be a size hint
		u8 isEncrypted;						//Only saw this affect encryption and linear image
		u8 specialTiling;					//Seems to be set when images uses different tiling
		u16 unknown;						//0x0000
		u32 tileDataSize;					//tileDataSize / 1024 = tileCount; tileDataSize * (2 - (tileDepth - 3)) = pixels
		u32 c_padding0;						//0x00000018
	};

	struct SOPC : GenericSection {												//Character info
		u32 c_padding;						//0x00000000
		u16 tileWidth;						//= tileCount in RAHC
		u16 tileHeight;						//= RAHC tileCount
	};

	typedef GenericResource<0x4E434752, false, RAHC, SOPC> NCGR;				//Graphics resource

	//NSCR (Tilemap file)

	struct NRCS : GenericSection {												//Screen resource
		u16 screenWidth;					//Width of screen (pixels)
		u16 screenHeight;					//Height of screen (pixels)
		u32 c_padding;						//unknown
		u32 screenDataSize;					//Size of screen data buffer
	};

	typedef GenericResource<0x4E534352, false, NRCS> NSCR;						//Screen resource

	//NARC (Archive)

	struct BTAF : GenericSection {												//File allocation table
		u32 files;
	};

	struct BTNF : GenericSection {};											//File name table
	struct GMIF : GenericSection {};											//File image

	typedef GenericResource<0x4352414E, false, BTAF, BTNF, GMIF> NARC;			//Archive file

	/*Model file

	struct MDL0 : GenericSection { };

	struct TEX0 : GenericSection {

		u32 p0;								//0x0

		u16 dataSize;						//<< 3
		u16 infoOffset;						//0x3C

		u32 p1;								//0x0

		u32 dataOffset;

		u32 p2;								//0x0

		u16 compressedSize;					//<< 3
		u16 compressedInfoOffset;			//0x3C

		u32 p3;								//0x0

		u32 compressedOffset;

		u32 compressedDataInfoOffset;

		u32 p4;								//0x0

		u32 paletteSize;					//<< 3

		u32 paletteInfoOffset;

		u32 paletteOffset;

	};

	typedef GenericResource<0x30444D42, true, MDL0, TEX0> BMD0;*/

	//All resource types

	typedef Buffer NBUO;

	typedef CompileTimeList<NCLR, NCGR, NSCR, NARC, NBUO> ResourceTypes;

	//Regular NDS info

	struct NDS {																//NDS file format
		c8 title[12];
		c8 gameCode[4];
		c8 makerCode[2];
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

	struct NDSTitle {

		WString title;
		WString subtitle;
		WString publisher;

		NDSTitle(const WString &title, const WString &subtitle, const WString &publisher) :
			title(title), subtitle(subtitle), publisher(publisher) {}

		NDSTitle() : NDSTitle(L"", L"", L"") {}
	};

	struct NDSBanner {

		enum class NDSTitleLanguage {
			JAPANESE,
			ENGLISH,
			FRENCH,
			GERMAN,
			ITALIAN,
			SPANISH
		};

		u16 version;
		u16 checksum;
		u8 reserved[28];
		u8 icon[512];	//NDS icon data (palette[i])
		u16 palette[16];	//NDS icon palette (BGR5)
		u16 titles[6][128];	//Title of game in 6 languages (unicode)

		//Get banner from NDS file
		static NDSBanner *get(NDS *nds);

		WString getTitle(NDSTitleLanguage lang);

	};

}