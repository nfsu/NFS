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
```
## Data Structure
For those of you who are interested; the NCGR uses the following struct (simplified):
```cpp
struct NCGR {

	//GenericHeader
	u32 magicNumber;				//MagicNumber; NCLR, NCGR, NSCR, etc.
	u32 c_constant;
	u32 size;					//Size of the header; including contents
	u16 c_headerSize;
	u16 sections;					//Number of sections
    
	//RAHC
	u16 tileHeight;					//Width in tiles (* 8)
	u16 tileWidth;					//Height in tiles (* 8)
	u16 tileDepth;					//1 << (tileDepth - 1) = bit depth
	u16 sizeHint0;					//0x0A or 0x00
	u16 sizeHint1;					//0x10 or 0x00, might be a size hint
	u16 sizeHint2;					//0x10 or 0x00, might be a size hint
	u8 isEncrypted;					//Only saw this affect encryption
	u8 specialTiling;				//Seems to be set when images uses different tiling
	u32 tileDataSize;				//tileDataSize / 1024 = tileCount; tileDataSize * (2 - (tileDepth - 3)) = pixels
	u32 c_padding0;					//0x00000018
	u8 rahc_data[size - sizeof(RAHC)];
    
	//SOPC (Not always present; only if sections > 1)
	u32 magicNumber;				//MagicNumber; TTLP, PMCP, etc.
	u32 size;					//Size of section; including contents
	u32 c_padding;				    	//0x00000000
	u16 tileWidth;				    	//= tileCount in RAHC
	u16 tileHeight;				    	//= RAHC tileCount
	u8 sopc_data[size - sizeof(SOPC)];
};
```
Note that the struct declared above is only for demonstration; it doesn't actually compile. I had to do some template magic to get this to work.
### Image structure
Like said before, NCGR is a tiled texture; this means that every 8x8 pixel block is stored; which means that you end up with a `PalettePtr[8][8][tilesX][tilesY]`; which makes sampling very difficult. PalettePtr can be a byte or a nibble; meaning that if the tileDepth == 3, it will use 4 bits per pixel. It also means that if you simply see it as a nibble array or byte array, that it will not look right. Most of the time, you have to convert linear image space to tiled image space, which is done automatically by NFS's Texture2D. If you get that value, you can then get the color from the palette. The nibble at 0xF is the x in the palette, The nibble at 0xF0 is the y in the palette (only 8 bit pixels have this).

### Special textures

Most times, the NCGR's data uses 8x8 tiling and specifies width and height. However, it can use special layouts that are either meant as a protection against modification (encryption) or could have been put in place as a special optimization for the NDS's hardware.

#### Encryption

The CHAR/RAHC section of the NCGR file contains the "isEncrypted" flag, which I've not seen in any other specification of the file format. I've encountered that whenever set, this determines that the texture uses a linear layout with encryption. Whenever reading it like a normal image, you'd get a noise image (very similar to what you'd get from a PRNG). If you interpret this as "isLinear" instead of "isEncrypted", it would still look like noise, even though it's in the correct layout.

The encryption used is either regular or reverse linear congruential encryption. A random number generator bound to the texture, that can be used to decrypt it.

```cpp
seed = texture[beg];
foreach it = beg...end
    magicTexture[i] = seed & 0xFFFF;
	seed = PRNG(seed);
```

Where texture is the texture as a `u16[]` and PRNG is the NDS's pseudo random number generator:

```cpp
return seed * 0x41c64e6d + 0x6073;
```

The "magic texture" (the decryption texture) is the `u16[]` that can be XOR-ed with the texture to get the actual result. I use this technique rather than XORing it directly, because I want to support native textures and not modify them in the ROM while displaying them.

##### Reverse or regular

The iterators given (beg, end) have to be determined. When reverse LCE is used, the beg is at the end of the array, while end is at the start. Regular LCE just places beg at the start and end at the end.

You can use the fact that the first and second value of the iterator have to be 0 to understand that you can determine whether or not regular or reverse encryption is used. 

```cpp
//*beg ^= *beg is always zero, so don't include it
u16 next = *(beg + 1);
u32 seed = CompressionHelper::generateRandom(*beg);
bool reverse = (next ^ seed) & 0xFFFF;
```

At the example above, we place 'beg' at data + 0. We know that XORing it by the seed (itself) will always return 0, so we don't check it. The second value however, will only be 0 if the decryption is valid.

The encryption immediately becomes invalid if you modify the first (or last) u16 (can be 4 or 2 colors) of the image. And because we require checking the value next to it for 0, we can't modify it either. *This is why I guarded these pixels from modification in the CPU-side of NFS*.

##### Code snippet

```cpp
u32 endInd = dataSize / 2 - 1;
u16 *beg = (u16*) data, *end = beg + endInd;

u16 next = *(beg + 1);
u32 seed = CompressionHelper::generateRandom(*beg);
bool reverse = ((next ^ seed) & 0xFFFFU) != 0;

i32 add = reverse ? -1 : 1;
i32 i = reverse ? endInd : 0;

seed = beg[i];

while ((reverse && i >= 0) || (!reverse && i <= endInd)) {
	u8 seed0 = seed & 0xFF;
	u8 seed1 = (seed & >> 8) & 0xFF;
	magic[i * 2] = seed0;
	magic[i * 2 + 1] = seed1;
	seed = CompressionHelper::generateRandom(seed);
	i += add;
}
```

I convert it to a u8 array so I can use it easier when sampling it on the GPU or CPU.

If you want to apply the magic texture to get the final result, XOR it:

```cpp
for(u32 i = 0; i < dataSize; ++i)
    data[i] ^= magic[i];
```

#### Size hints

Unfortunately, tileWidth and tileHeight in RAHC can't be trusted. These values can be set to u16_MAX (0xFFFF) and then the width/height can't be determined. However, the size hints might provide more insight into what the size might actually be.

sizeHint0 is rarely set to anything, but I once saw this value as non zero. 0x0A IIRC. Some specifications say that this is part of the paletteDepth, though they mention that paletteDepth can only be 3 (4-bit) and 4 (8-bit).

sizeHint2 seems to only be set to 0x10 if sizeHint1 is set to 0x10. Maybe this is used with a height table?

##### sizeHint1

Is still not completely logical to me, though I have managed to compose a table that seems to return the width for a lot of different images (using the size of the image as input):

```cpp
std::unordered_map<u16, u32> widthTable = {
    { 384, 16 },
    { 768, 16 },
    { 2048, 32 },	//Not always correct
	{ 6400, 64 },	//Not always correct
    { 9216, 32 },
    { 16384, 64 }
};
```

The height can be calculated by dividing the size of the image by the width.

This might internally use a function instead of using a table, though I wasn't able to figure out what. The closest match I got was `32 * (1 + (size >> 14));` but that doesn't match `widthTable[0]`.

#### Special tiling

Not exactly sure what this flag does, but it seems to be turned on in images in trgra/trbgra.narc/0.NCGR. When this flag is set, it seems that sizeHint1 and sizeHint2 are set to 0x10. When this flag is set, no clear way of decoding the image has been found (yet).

This flag could also just be another size hint.

Maybe this is "isAnimated"? and then sizeHint2 is being used as animation width & height? Maybe if 0x20 sizeHint2