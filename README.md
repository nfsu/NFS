# NFS ![Logo](https://i.imgur.com/bBy2kS9.png)
Nintendo File System
NFS(U) stands for Nintendo File System (Utils) and is designed to read and interpret nds files such as NARC, NCLR, NCGR, etc.
![Logo](https://i.imgur.com/PWpc5ZX.png)
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
The reason this API is so fast is it never mallocs; all resources are structs or use the ROM buffer. This means that the rom is directly affected if you write to a GenericResource's buffer (or convert it to things like textures). The following is how you modify a 64x64 image:
```cpp
	Texture2D tex;
	NType::convert(ncgr, &tex);

	for (u32 i = 0; i < 64; ++i)
		for (u32 j = 0; j < 64; ++j) {
			f32 deltaX = i - 31.5f;
			f32 deltaY = j - 31.5;
			deltaX /= 31.5;
			deltaY /= 31.5;
			f32 r = sqrt(pow(deltaX, 2) + pow(deltaY, 2));
			setPixel(tex, i + 0, j + 0, 0x1 + r * 0xE);
		}
```
'setPixel' doesn't just set the data in the texture; it also checks what kind of texture is used. If you are using a BGR555 texture, you input RGBA8 but it has to convert it first. 'getPixel' does the same; except it changes the output you receive. If you want the direct info, you can use fetchData or storeData; but it is not recommended.
###  Writing textures
Textures aren't that easy in NFS; palettes are always used and sometimes, they even use tilemaps. This means that fetching the data directly won't return an RGBA8 color, but rather an index to a palette or tile. If you want to output the actual image, you can create a new image that will read the ROM's image:
```cpp
	Texture2D tex2 = convertToRGBA8({ tex.width, tex.height, palette, tex });
	writeTexture(tex2, "Final0.png");
	deleteTexture(&tex2);```
Don't forget to delete the texture; as it uses a new malloc, because most of the time, the format is different from the source to the target. 
### Writing image filters
If you'd want to add a new image filter, I've created a helpful function, which can be used as the following:
```cpp
Texture2D convertToRGBA8(PaletteTexture2D pt2d) {
	return runPixelShader<PaletteTexture2D>([](PaletteTexture2D t, u32 i, u32 j) -> u32 { 
		u32 sample = getPixel(t.tilemap, i, j);
		u32 x = sample & 0xF;
		u32 y = (sample & 0xF0) >> 4;
		return getPixel(t.palette, x, y);
	}, pt2d);
}
```
This function is called 'runPixelShader', which can be applied to any kind of object (default is Texture2D). It will expect width and height in the struct and will loop through all indices in the 'image'. In our case, you can use it to convert a PaletteTexture2D to a Texture2D; since it just reads the tilemap and finds it in the palette.
RunPixelShader will however return a new texture and will put it into RGBA8 format.
## Special thanks
Thanks to /LagMeester4000 for creating magic templates that are used all the time in this API. Typelists are used from his repo at [/LagMeester4000/TypeList](https://github.com/LagMeester4000/TypeList).
## Nintendo policies
Nintendo policies are strict, especially on fan-made content. If you are using this to hack roms, please be sure to NOT SUPPLY roms and if you're using this, please ensure that copyright policies are being kept. I am not responsible for any programs made with this, for it is only made to create new roms or use existing roms in the fair use policy.
## Fair use
You can feel free to use this program any time you'd like, as long as you mention this repo.
