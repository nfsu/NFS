# NDS basics
When I started modding, I couldn't find a lot of information on Nintendo's file system. 
The info I found was either lacking, incorrect, missing valuable information and/or hard to wrap your head around.
So I decided to document the basics of NDS hacking.
## NDS concept
Back when NDS was used, there wasn't a lot of space. So most of the time, you'd have 128->256 MiB ROMs.
Those ROMs contain a lot of data; the palettes, tilemaps, maps, models, animations, files, folders, code and more data.
It is basically like 'zipping' a folder, it keeps the names of the files and folders and the data.
For parsing this, you'd need a 'header', which is the important information you need to know before parsing a file. 
It tells you where certain things are located and general information.
## NDS header
The following is the NDS header;
```cpp
  //NDS file format
	struct NDS {
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
```
(Where u<x> is defined as an unsigned <x> bit integer)
This contains a lot of information; now let's start with the basic information of this file; file information.
The variables ftable_off and ftable_len are used for determining the position of the file table and the length. 
This table contains the relationships between folders and files and their names.
This means that we know where the buffer is located; at [nds->ftable_off, nds->ftable_off + nds->ftable_len>.
That buffer will contain the following types of data (psuedo code):
```cpp
  struct FolderInfo {
    u32 offset;
    u16 firstFilePosition;
    u16 relation;
  };
  
  struct FileName {
    u8 len;
    char string[?];
  };
  
  struct FNT {
    FileInfo info[];
    FileName names[];
  };
```
The first folder's relation will be somewhere in range [0x0, 0xFFF], this is the total number of folders. 
All folders onwards use a relation of [0xF000, 0xFFFF], where the last 3 nibbles indicate which folder this folder is located in.
