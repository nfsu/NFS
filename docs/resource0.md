# NCLR - Nintendo CoLour Resource
The NCLR file is a palette resource, it contains the pixels that an image will use (NCGR or NSCR mostly use them).
## Obtaining an NCLR file
To obtain an NCLR file, you parse an NDS file and search for an NCLR file you want to open. Then, you use the following function:
```cpp
  NCLR &nclr = fileSystem.get<NCLR>(path);
```
And now, you can access TTLP/PMCP (the sections), through at<0/1> and their data through get<0/1>. However, most of the time, you don't need that.
```cpp
  TTLP &ttlp = nclr.at<0>();    //Palette data
  PMCP &pmcp = nclr.at<1>();    //Palette count map (can be null)
  Buffer ttlpd = nclr.get<0>(); //TTLP's data
  Buffer pmcpd = nclr.get<1>(); //PMCP's data
```
You can also get the generic header; however, this only works on GenericResources and so NBUO (Buffer Unknown Object) won't work.
```cpp
  GenericResource &gr = *nclr.header;
```
From that, you can get the size of the NCLR.
## Using an NCLR
If you don't want to use that data, you can always convert it to a more logical resource. Note that not all resource conversions come for 'free'; but NCLR's conversion does.
```cpp
  Texture2D tex(nclr);
```
Afterwards, you can use 'tex' like you would use any texture. You can get the width/height and use the read function to read pixels.
## Data structure
For those of you who are interested; the NCLR uses the following struct (simplified):
```cpp
struct NCLR {

	//GenericHeader
	u32 magicNumber;					//MagicNumber; NCLR, NCGR, NSCR, etc.
	u32 c_constant;
	u32 size;						//Size of the header; including contents
	u16 c_headerSize;
	u16 sections;						//Number of sections
    
	//TTLP
	u32 magicNumber;					//MagicNumber; TTLP, PMCP, etc.
	u32 size;						//Size of section; including contents
	u32 bitDepth;						//3 = 4 bits, 4 = 8 bits
	u32 c_padding;						//0x00000000
	u32 dataSize;						//size of palette data in bytes; if(size > 0x200) size = 0x200 - size;
	u32 c_colors;						//0x00000010
	char ttlp_data[size - sizeof(TTLP)];
    
	//PMCP (Not always present; only if sections > 1)
	u32 magicNumber;					//MagicNumber; TTLP, PMCP, etc.
	u32 size;						//Size of section; including contents
	u16 count;						//Count of palettes in file
	u32 c_constant;						//0xEFBE08
	u16 c_padding;						//0x0000
	char pmcp_data[size - sizeof(PMCP)];
};
```
Note that the struct declared above is only for demonstration; it doesn't actually compile. I had to do some template magic to get this to work.
### Palette data
Since NDS files are quite old, they used BGR5; a 5 bit per channel color system, giving us 32k colors instead of 16M. This means that you have to convert the texture before you do anything with it; however, in NFS, Texture's read/write function convert the data for you.
