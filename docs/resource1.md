# NCGR - Nintendo Character Graphics Resource
The NCGR file is a tilemap resource, it contains the stores an image; where all values are indices into a palette. This resource can be 'tiled' and tiles can be used by an NSCR file (and mirrored / palette swapped) for optimization.
## Obtaining an NCGR file
To obtain an NCGR file, you parse an NDS file and search for an NCGR file you want to open. Then, you use the following function:
```cpp
  NCGR &ncgr = fileSystem.get<NCGR>(path);
```
And now, you can access RAHC/SOPC (the sections), through at<0/1> and their data through get<0/1>. However, most of the time, you don't need that.
```cpp
  RAHC &rahc = ncgr.at<0>();    //Image description
  SOPC &sopc = ncgr.at<1>();    //Image info (can be null)
  Buffer rahcd = ncgr.get<0>(); //RAHC's data
  Buffer sopcd = ncgr.get<1>(); //SOPC's data
```
You can also get the generic header; however, this only works on GenericResources and so NBUO (Buffer Unknown Object) won't work.
```cpp
  GenericResource &gr = *ncgr.header;
```
From that, you can get the size of the NCGR.
## Using an NCGR
Using an NCGR can be done in different ways;
- Using the original data
- Using a converted copy
Since the NCGR references a palette, for converting you need a NCLR file. This is the corresponding palette file and is required for converting it into a texture. If you don't want to convert (because if you change the copy, you don't change the ROM's version), then you can simply invoke Texture2D's constructor with NCGR as first argument.
```cpp
  Texture2D directROMAccess(ncgr);
  Texture2D convertedCopy(ncgr, nclr);
  convertedCopy.dealloc();
```
Don't forget that the second line creates a new texture and so dealloc has to be called.
## Data Structure
For those of you who are interested; the NCGR uses the following struct (simplified):
```cpp
struct NCGR {

	//GenericHeader
	u32 magicNumber;					//MagicNumber; NCLR, NCGR, NSCR, etc.
	u32 c_constant;
	u32 size;						      //Size of the header; including contents
	u16 c_headerSize;
	u16 sections;						  //Number of sections
    
	//RAHC
	u16 tileHeight;
  u16 tileWidth;
  u32 tileDepth;					  //1 << (tileDepth - 1) = bit depth
  u64 c_padding;					  //0x0000000000000000
  u32 tileDataSize;				  //tileDataSize / 1024 = tileCount; tileDataSize * (2 - (tileDepth - 3)) = pixels
  u32 c_padding0;					  //0x00000018
	char ttlp_data[size - sizeof(RAHC)];
    
	//SOPC (Not always present; only if sections > 1)
	u32 magicNumber;					//MagicNumber; TTLP, PMCP, etc.
	u32 size;						      //Size of section; including contents
  u32 c_padding;				    //0x00000000
  u16 tileWidth;				    //= tileCount in RAHC
  u16 tileHeight;				    //= RAHC tileCount
	char pmcp_data[size - sizeof(SOPC)];
};
```
Note that the struct declared above is only for demonstration; it doesn't actually compile. I had to do some template magic to get this to work.
### Image structure
Like said before, NCGR is a tiled texture; this means that every 8x8 pixel block is stored; which means that you end up with a `PalettePtr[8][8][tilesX][tilesY]`; which makes sampling very difficult. PalettePtr can be a byte or a nibble; meaning that if the tileDepth == 3, it will use 4 bits per pixel. It also means that if you simply see it as a nibble array or byte array, that it will not look right. Most of the time, you have to convert linear image space to tiled image space, which is done automatically by NFS's Texture2D. If you get that value, you can then get the color from the palette. The nibble at 0xF is the x in the palette, The nibble at 0xF0 is the y in the palette (only 8 bit pixels have this).
