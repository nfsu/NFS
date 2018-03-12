# NSCR - Nintendo SCreen Resource
The NCSCR file is a map resource, it is a resource that describes where what tiles are stored, if they are mirrored and what their palette is.
## Obtaining an NSCR file
To obtain an NSCR file, you parse an NDS file and search for an NSCR file you want to open. Then, you use the following function:
```cpp
  NSCR &nscr = fileSystem.get<NSCR>(path);
```
And now, you can access NCRS (the section), through at<0> and the data through get<0>. However, most of the time, you don't need that.
```cpp
  NCRS &ncrs = nscr.at<0>();    //Screen resource
  Buffer ncrsb = nscr.get<0>(); //NCRS's data
```
You can also get the generic header; however, this only works on GenericResources and so NBUO (Buffer Unknown Object) won't work.
```cpp
  GenericResource &gr = *nscr.header;
```
From that, you can get the size of the NSCR.
## Using an NSCR
Using an NSCR can be done in different ways;
- Using the original data
- Using a converted copy
Since the NSCR references a palette and tilemap, for converting you need a NCLR/NCGR file. It is required for converting it into a texture. If you don't want to convert (because if you change the copy, you don't change the ROM's version), then you can simply invoke Texture2D's constructor with NSCR as first argument.
```cpp
  Texture2D directROMAccess(nscr);
  Texture2D convertedCopy(nscr, ncgr, nclr);
  convertedCopy.dealloc();
```
Don't forget that the second line creates a new texture and so dealloc has to be called.
## Data structure
For those of you who are interested; the NSCR uses the following struct (simplified):
```cpp
struct NSCR {

	//GenericHeader
	u32 magicNumber;			        //MagicNumber; NCLR, NCGR, NSCR, etc.
	u32 c_constant;
	u32 size;						          //Size of the header; including contents
	u16 c_headerSize;
	u16 sections;					        //Number of sections
    
	//NCRS
  u16 screenWidth;			        //Width of screen (pixels)
  u16 screenHeight;			        //Height of screen (pixels)
  u32 c_padding;				        //unknown
  u32 screenDataSize;		        //Size of screen data buffer
	char ncrs_data[size - sizeof(NCRS)];
};
```
Note that the struct declared above is only for demonstration; it doesn't actually compile. I had to do some template magic to get this to work.
### Image data
The layout of NCRS is a u16 per pixel and the following is contained per pixel:
```
Tile position at 0x3FF
Tile mirrored? at 0xC00 (& 0x800 = mirrorY, & 0x400 = mirrorX)
Tile palette at 0xF000
```
Tile position is the position in the tilemap; the following is how you calculate where the tile is positioned
```cpp
  if (mirrorX)
		pixInTileX = (tiles - 1) - pixInTileX;

	if (mirrorY)
		pixInTileY = (tiles - 1) - pixInTileY;

	u32 tilemapX = (tilePos % tilesX) * tiles + offX;
	u32 tilemapY = (tilePos / tilesX) * tiles + offY;
```
