#pragma once
#include "GenericResource.h"

namespace nfs {

	struct TTLP : public GenericSection {										//palette data
		u32 bitDepth;						//3 = 4 bits, 4 = 8 bits
		u32 c_padding;						//0x00000000
		u32 dataSize;						//size of palette data in bytes; if(size > 0x200) size = 0x200 - size;
		u32 c_colors;						//0x00000010
	};

	struct PMCP : public GenericSection {										//Palette count map
		u16 count;							//Count of palettes in file
		u32 c_constant;						//0xEFBE08
		u16 c_padding;						//0x0000
	};

	typedef GenericResource<0x4E434C52, TTLP, PMCP> NCLR;						//Palette ("Color") resource

	struct RAHC : GenericSection {												//Character data
		u16 tileHeight;
		u16 tileWidth;
		u32 tileDepth;					//1 << (tileDepth - 1) = bit depth
		u64 c_padding;					//0x0000000000000000
		u32 tileDataSize;				//tileDataSize / 1024 = tileCount; tileDataSize * (2 - (tileDepth - 3)) = pixels
		u32 c_padding0;					//0x00000018
	};

	struct SOPC : GenericSection {												//Character info
		u32 c_padding;				//0x00000000
		u16 tileWidth;				//= tileCount in RAHC
		u16 tileHeight;				//= RAHC tileCount
	};

	typedef GenericResource<0x4E434752, RAHC, SOPC> NCGR;						//Graphics resource

	struct NRCS : GenericSection {												//Screen resource
		u16 screenWidth;				//Width of screen (pixels)
		u16 screenHeight;				//Height of screen (pixels)
		u32 c_padding;					//unknown
		u32 screenDataSize;				//Size of screen data buffer
	};

	typedef GenericResource<0x4E534352, NRCS> NSCR;								//Screen resource

	struct BTAF : GenericSection {												//File allocation table
		u32 files;
	};

	struct BTNF : GenericSection {};											//File name table
	struct GMIF : GenericSection {};											//File image

	typedef GenericResource<0x4352414E, BTAF, BTNF, GMIF> NARC;					//Archive file

	typedef Buffer NBUO;

	typedef CompileTimeList<NCLR, NCGR, NSCR, NARC, NBUO> ResourceTypes;

	struct NDS {																//NDS file format
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

}