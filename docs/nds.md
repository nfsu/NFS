# NDS - Nintendo Dual Screen
The NDS format contains a few things; one of the most important ones are a file table and code.
## NDS Header
```cpp
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
		u8 flags;			//Flags such as auto start (Bit2)
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
		u16 sAC;			//Secure area checksum
		u16 sALT;			//Secure area loading timeout
		u32 arm9_ALLRA;			//Auto load list RAM address
		u32 arm7_ALLRA;			//Auto load list RAM address
		u64 sAD;			//Secure area disable
		u32 romSize;
		u32 romHeaderSize;
		u8 reserved1[56];
		u8 nLogo[156];
		u16 nLC;			//Logo Checksum
		u16 nHC;			//Header Checksum
		u32 dRomOff;			//Debug rom offset
		u32 dRomSize;			//Debug rom size
		u32 reserved2;
		u8 reserved3[144];
	};
```
As described above, it contains various locations in the ROM, which can be used to run the ROM or find the resources.
## Parsing the file system
The FNT (File Name Table) contains the following data:
```cpp
struct FolderInfo {
	u32 offset;
	u16 firstFilePosition;
	u16 relation;
};
```
Every folder has this struct and to find out how many folders there are, you jump into the root directory (FolderInfo at 0) and get the 'relation'. The root's 'firstFilePosition' is the offset which has to be added to a file's index to get the buffer through the file allocation table.
```cpp
	FolderInfo *folderArray = (FolderInfo*)fnt.ptr;
	FolderInfo &root = *folderArray;

	if ((root.relation & 0xF000) != 0)
		throw std::exception("FileSystem Couldn't find root folder");

	u16 rootFolders = root.relation;
	u16 startFile = root.firstFilePosition;

	if (fnt.size < sizeof(FolderInfo) * rootFolders)
		throw std::exception("FileSystem Out of bounds exception");
    
	for (u16 i = 0; i < rootFolders; ++i) {
		fileSystem[i].index = i;
		fileSystem[i].resource = i == 0 ? u16_MAX : u16_MAX - 1;
		fileSystem[i].parent = i == 0 ? u16_MAX : folderArray[i].relation & 0xFFF;
		fileSystem[i].fileHint = fileSystem[i].folderHint = u16_MAX;

		if (i == 0)
			fileSystem[i].name = "/";
	}
```
As you can see above, the names aren't specified, that's because they come after the array of FolderInfo. The name data is laid out as following:
```
u8 var (isFolder = var & 0x80; nameLength = var & 0x7F)
```
If however, var is 0x00, it means that it wants to jump to the next folder. For this, we use a 'current folder' variable, which indicates to which folder the object belongs. This starts out at 0; the root file. But when it encounters the next 0x00, it will increase it and jump to the first file in the directory. If that's not the case, it will follow with a char[nameLength], representing the name of the next object. Every file, the file offset increases and that variable 'fileOffset' is used to determine the buffer of that file.
```cpp
	u32 &beg = *(u32*)(fpos.ptr + fileOffset * 8);
	u32 &end = *(u32*)(fpos.ptr + fileOffset * 8 + 4);
	u32 len = end - beg;

	fso.buf = { len, ((u8*)rom) + beg };
```
