# NFS
NFS(U) stands for Nintendo File System (Utils) and is designed to read and interpret nds files such as NARC, NCLR, NCGR, etc.
## Why NFS?
As a kid I loved modifying existing roms for my personal use, but tools were/are either extremely lacking (both in use and looks). I finally decided to look into Nintendo's file system and how things could be written / read. Thanks to template magician /LagMeester4000, I 'simplified' this system using C++ templates, allowing you to add custom formats within a few seconds. The reading/writing/converting is very quick and can be easily used in existing applications.
## How to use
Down below you can see a simply use of the NFS API.
### Reading resources
```cpp
  	NCLR nclr;
	NType::readGenericResource(&nclr, offset(buf, NCLR_off));

	NCGR ncgr;
	NType::readGenericResource(&ncgr, offset(buf, NCGR_off));

	NARC narc;
	NType::readGenericResource(&narc, offset(buf, NARC_off));
```
NType::readGenericResource is a function that is built to read a GenericResource; which is the base of all Nintendo resources.
### Converting resources
```cpp
  	NArchieve arch;
	NType::convert(narc, &arch);
```
NType::convert is created to wrap around existing resources; it could convert a NCLR (palette) to a 2D texture, a NARC to NArchieve and more.
### Adding a resource type
```cpp
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
```
A NARC is defined as a resource containing a BTAF, BTNF and GMIF. The GMIF contains the archieve buffer, BTNF contains the offsets and buffer lengths.
You also need to add the following code to MagicNumber and SectionLength; so the API can detect your custom file type and knows the sizes of the buffers for the sections.
```cpp
//In MagicNumber
		template<> constexpr static u32 get<GMIF> = 0x46494D47;
		template<> constexpr static u32 get<BTAF> = 0x46415442;
		template<> constexpr static u32 get<NARC> = 0x4352414E;
		template<> constexpr static u32 get<BTNF> = 0x464E5442;
//In SectionSize
  template<> constexpr static u32 get<GMIF> = 8;
		template<> constexpr static u32 get<BTAF> = 12;
		template<> constexpr static u32 get<BTNF> = 8;
```
### Writing a resource type
TODO:
## Special thanks
Thanks to /LagMeester4000 for creating magic templates that are used all the time in this API. Typelists are used from his repo at [/LagMeester4000/TypeList](https://github.com/LagMeester4000/TypeList).
## Nintendo policies
Nintendo policies are strict, especially on fan-made content. If you are using this to hack roms, please be sure to NOT SUPPLY roms and if you're using this, please ensure that copyright policies are being kept. I am not responsible for any programs made with this, for it is only made to create new roms or use existing roms in the fair use policy.
## Fair use
You can feel free to use this program any time you'd like, as long as you mention this repo.
