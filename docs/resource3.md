# NARC - Nintendo Archive
NARC is an archive file; meaning that it contains a list of files. These files are unnamed, which makes it harder to find out what they are.
## Obtaining a NARC file
To obtain a NARC file, you parse an NDS file and search for a NARC file you want to open. Then, you use the following function:
```cpp
  NARC &narc = fileSystem.get<NARC>(path);
```
And now, you can access BTAF/BTNF/GMIF (the sections), through at<0/1/2> and their data through get<0/1/2>. However, most of the time, you don't need that.
```cpp
  BTAF &btaf = narc.at<0>();    //File alloc table
  BTNF &btnf = narc.at<1>();    //File name table
  GMIF &gmif = narc.at<2>();    //File image
  Buffer btafd = narc.get<0>(); //BTAF's data
  Buffer btnfd = narc.get<1>(); //BTNF's data
  Buffer gmifd = narc.get<2>(); //GMIF's data
```
You can also get the generic header; however, this only works on GenericResources and so NBUO (Buffer Unknown Object) won't work.
```cpp
  GenericResource &gr = *narc.header;
```
From that, you can get the size of the NARC.
## Using a NARC
Converting a NARC takes time; as it has to read the file data and convert it to a faster format. When you convert it, make sure you really have to. FileSystem already parses subresources for you, so using this function is mostly useless, unless you don't have a NDS file it goes into.
```cpp
  Archive a(narc);
```
## Data structure
For those of you who are interested; the NARC uses the following struct (simplified):
```cpp
struct NARC {

	//GenericHeader
	u32 magicNumber;					//MagicNumber; NCLR, NCGR, NSCR, etc.
	u32 c_constant;
	u32 size;						//Size of the header; including contents
	u16 c_headerSize;
	u16 sections;						//Number of sections
    
	//BTAF
	u32 magicNumber;					//MagicNumber; TTLP, PMCP, etc.
	u32 size;						//Size of section; including contents
	u32 files;
	u8 btaf_data[size - sizeof(BTAF)];
    
	//BNTF
	u32 magicNumber;					//MagicNumber; TTLP, PMCP, etc.
	u32 size;						//Size of section; including contents
	u8 btnf_data[size - sizeof(BNTF)];
  
	//GMIF
	u32 magicNumber;					//MagicNumber; TTLP, PMCP, etc.
	u32 size;						//Size of section; including contents
	u8 gmif_data[size - sizeof(GMIF)];
};
```
Note that the struct declared above is only for demonstration; it doesn't actually compile. I had to do some template magic to get this to work.
### Getting the files
The BTAF contains a `u32[2 * files]` that specifies the begin/end of every file. GMIF's data contain all the files, so gmif_data + begin is the start of the file, while gmif_data + end is the end of the file. 
