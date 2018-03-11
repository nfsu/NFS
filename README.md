![NFS logo](https://i.imgur.com/uabtvGW.png)
<br>
Nintendo File System
-------

NFS(U) stands for Nintendo File System (Utils) and is designed to read and interpret nds files such as NARC, NCLR, NCGR, etc.  
# But why?
Some time ago, I loved modifying existing roms for my personal use, but tools were/are either extremely lacking (both in use and looks) or they frequently crashed and were a pain to use. I finally decided to look into Nintendo's file system and how things could be written / read. Thanks to template magician [LagMeester4000](https://github.com/LagMeester4000), I 'simplified' this system using C++ templates, allowing you to add custom formats within a few seconds. The reading/writing/converting is very quick and can be easily used in existing applications.
# How to install
The newest version of NFS uses cmake to generate the build projects for you, you can simply run 'reload.bat' and it will generate the files for you. With this, you can add new projects and depend on NFS/NFSU. Simply add the following cmake file (CMakeLists.txt) in your project <name>.
```cmake
include_directories(../nfs/include)
include_directories(include)

file(GLOB_RECURSE <name>_SRC
	"include/*.h"
	"src/*.c"
	"include/*.hpp"
	"src/*.cpp"
)

add_executable(
	<name>
	${<name>_SRC}
)

target_link_libraries(<name> nfs)
```
And add it to the list in the root cmake
```cmake
add_subdirectory(nfs)
add_subdirectory(nfsu)
add_subdirectory(<name>)
```
	
## How to use
Down below you can see a simple use of the NFS API.
### Reading raw ROM data
```cpp
	using namespace nfs;
	Buffer buf = Buffer::read("ROM.nds");
	try {
		NDS *nds = (NDS*) buf.ptr;
		FileSystem fs(nds);
	} catch (std::exception e) {
		printf("%s\n", e.what());
	}
	buf.dealloc();
```
If you've acquired your ROM, you have to load it into memory, so the program can read and modify it. This buffer can be deleted once you don't want to use the FileSystem anymore.  
NDS is a header file with the most important information about the ROM; such as, where the code and files are located and the name of the ROM.
### Converting the ROM into a FileSystem
A ROM is like a ZIP; except it is used for storing game data (models, images, sounds, palettes, maps, binary data, text, code and more). This means that it stores the file names into the ROM; which we can use to extract the files we need and where they are. Above, you could see that converting to a FileSystem is done by simply using the constructor; so even fs = nds; is okay.
This will put the file data into the fs variable, which you can then loop through and use.
```cpp
		for (auto iter = fs.begin(); iter != fs.end(); ++iter) {

			FileSystemObject &fso = *iter;
			//More code...
		}
```
### Checks for fso's
Fso stands for 'FileSystemObject' and it is what I call folders and files; this means that fso isn't always a file, it could also be a folder. To distinguish them, you can use the following functions:
- isFile
- isFolder
- isRoot
- hasParent  
Before converting a file, make sure it is actually a file. The other variables are explained as following:
- folders; the count of folders for this file
- files; the count of files for this file  (Yes; even files can have subfiles, as .NARC and .CARC are still a file, but contain files)
- objects; the count of objects for this file (folders + files)
- index; the index in the FileSystem
- parent; the index to the parent in the FileSystem
- resource; the index to the resource in the FileSystem's Archive
- fileHint; a hint about where the files are located (if it has any; otherwise it's u32_MAX)
- folderHint; a hint about where the folders are located (if it has any; otherwise it's u32_MAX)
- name; the absolute path to the directory
- buf; the buffer of the file, if it isn't a file, it's a null buffer
### Obtaining the resource of the file
```cpp
			if (fso.isFile()) {

				ArchiveObject &ao = fs.getResource(fso);
				if(ao.info.magicNumber == ResourceHelper::getMagicNumber<NBUO>()) {
					//It is not supported
				} else {	
					//It is supported
					printf("Supported object: %s (%s)\n", ao.name.c_str(), ao.info..c_str());
					
					if(ao.info.magicNumber == NCLR::getMagicNumber()){
						NCLR &nclr = fs.get<NCLR>(ao);
						u32 nclrSize = nclr.header->size;
						//More stuff
					}
				}
			}
			else {
				//It's not a file, but a folder or root folder
			}
```
Here we are getting the ArchiveObject; which is the physical representation of a file/folder, it tells you where what kind of resource is located. One identifier you can use is the 'magicNumber'; which is the standard way, you can also use the type, which is used by a few ResourceHelper functions and the name (extension), however, magicNumber is the fastest in most cases. If you are sure the fso is actually an object that you can read, you can use the FileSystem's Archive's 'get' function, which requires a template parameter. If you cast incorrectly, it will throw an std::exception, so you could try & catch it instead of checking first, but that's bad practice. Then, you can use the GenericHeader*, which has a size and a few other variables, or you can use the `at<i>()` method for getting a section. NCLR for example has a TTLP (Palette data) and sometimes a PMCP (Palette count map). These are defined in 'ntypes.h'. If you want to get the palette data, you can use get<0>, to get the buffer which contains the data for that section.
	
	

### Traversing a folder
'Traversing' is what I call searching through an entire folder; so traverse through /someFolder/something would give all of the folders and files in the directory and inside those folders. While the square brackets (or array operator) are used for obtaining folders/files directly inside a folder:
```cpp
	std::vector<const FileSystemObject*> fsos = files.traverse(files["fielddata"]);
	for (u32 i = 0; i < fsos.size(); ++i)
		printf("%s\n", fsos[i]->name.c_str());
```
`files["x"]` will get the fso at the position of x; that FSO can then be used to trace all files or get the files inside of that folder (if it is a folder). You could also traverse root; by using "/" as the folder, or simply by using `files[fso]`.
```cpp
	std::vector<const FileSystemObject*> fsos = files[files["/"]];
	for (u32 i = 0; i < fsos.size(); ++i)
		printf("%s\n", fsos[i]->name.c_str());
```
### Reading resources
Before you can use a resource, you need to convert them to the type you want to use. You do this the same way as with Archive/FileSystem; you use the constructor of the type you want to convert to. For example:
```cpp
	NARC &narc = fs.get<NARC>(fs.getResource(*fs["a/b/test.NARC"]));
	Archive arc(narc);
```
Above, you fetch the fso at "a/b/test.NARC", which could be nullptr (so you should only dereference when you're sure it's not). Then, you get that resource, which can be used to get the NARC. Then, you can convert it to an Archive; however, this should be prevented and only done when you REALLY need it. The FileSystem automatically parses NARC/CARCs inside of the file system, so there would be no need for this, seeing as it isn't a 'free' conversion, it takes a little time. There are however, conversions that do come for 'free' (as they are just detecting a type and not parsing anything). These include, but are not limited to, texture conversions (palette/tilemap/map).
```cpp
	Texture2D tex(nclr);
	PT2D tilemap(ncgr, nclr);
	PTT2D map(nscr, ncgr, nclr);
```
'Texture2D' is a simple 2D texture (could be any format; but in NDS it's BGR555 most of the time), 'PT2D' is Palette Texture 2D; so you have a palette linked to a texture, 'PTT2D' is Palette Tile Texture 2D, which is linked to a tilemap, which is linked to a palette.
### FileSystem's parent
FileSystem is a unique object, it has a folder structure. But, it still remains a list of resources. This is why it uses NArchive as its parent. It stores both resources and file information. This means that you can use the archive's functions too, but those can't be used in combination with file names.
## (The following is from previous documentation and isn't implemented yet)
### Archives
As said before, an archive is basically a list of resources, which you can get types of and cast.
```cpp
	std::string name = arch.getTypeName(i);
	u32 magicNumber = arch.getType(i);
```
The typeName and magicNumber aren't always correct, as an archive doesn't know about the extension of a file. It just knows that the first 4 bytes generally indicate the type of the file. You can't always rely on the typeName, but the magicNumber is valid (most of the time).
```cpp
	if (arch.getType(i) == MagicNumber::get<NCLR>)
		NCLR &nclr = arch.operator[]<NCLR>(i);
```
It could also be done by using a try and catch;
```cpp
	try {
		NCLR &nclr = arch.operator[]<NCLR>(i);
		Texture2D someTex;
		NType::convert(nclr, &someTex);
		writeTexture(someTex, "SomeTex.png");
	} catch (std::exception e) {}
```
This all means that you can simply loop through the archieve like an std::vector and use the types how you want:
```cpp
	for (u32 i = 0; i < arch.size(); ++i) {

		printf("%u %s %u\n", i, arch.getTypeName(i).c_str(), arch.getType(i));
		
		try {
			NCLR &nclr = arch.operator[]<NCLR>(i);
			printf("Palette with %u colors!\n", nclr.contents.front.c_colors);
		} catch (std::exception e) {

		}
	}
```
#### Flaws in resource reading
Resource reading interprets the data of the ROM into a struct; which means that some data can't be interpreted when you try loading them. This will create a struct that can't be written or read and only exists when you run convert on a NARC file. This is called an 'NBUO' (Buffer Unknown Object) and it contains one section; 'NBIS' (Buffer Info Section). All variables except the NBIS's Buffer are 0. So you can still interpret unknown file formats yourself.
```cpp
	try {
		NBUO &nclr = arch.operator[]<NBUO>(i);
		printf("Undefined object at %u with size %u\n", i, nclr.contents.front.data.size);
	}
	catch (std::exception e) {}
```
The NBUO has a magicNumber of 0; making it undefined and so does its section. It is only so you can read the data in there:
```cpp
	try {
		NBUO &nclr = arch.operator[]<NBUO>(i);
		u32 magicNum = *(u32*)nclr.contents.front.data.data;
		///Check if the magicNumber is actually a format that is implemented and interpret the buffer.
	}
	catch (std::exception e) {}
```
This is done so you can still edit file formats that might not be a standard, but are used in some ROMs.
### Adding a custom resource type
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
Afterwards, they also need to be inserted into the ArchieveTypes array, so the auto parser for the NArchive / FileSystem can recognize them.
```cpp
	//TypeList with all the archive types
	typedef lag::TypeList<NARC, NCSR, NCGR, NCLR, NBUO> ArchiveTypes;
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
	deleteTexture(&tex2);
```
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
### Running the example
Source.cpp is what I use to test if parts of the API work, however, I can't supply all dependencies. It is illegal to upload roms, so if you want to test it out, you have to obtain a rom first. Afterwards, you can use something like nitro explorer to find offsets of palettes, images, animations, models or other things you might use. All important things in Source.cpp have been suffixed by '//TODO: !!!', so please fix those before running.
## Supported file formats
File extension | File name | Magic number | Data type | Conversion type | isResource
--- | --- | --- | --- | --- | ---
NCLR | CoLor Resource | 0x4E434C52 | Palette | Texture2D | yes
NCGR | Character Graphics Resource | 0x4E434752 | Tilemap / image | Texture2D / PaletteTexture2D | yes
NSCR | SCreen Resource | 0x4E534352 | Map | Texture2D / TiledTexture2D | yes
NARC | Archive | 0x4352414E | Archive | NArchive | yes
NDS | Dual Screen | - | File system and code | FileSystem | no
## Special thanks
Thanks to /LagMeester4000 for creating magic templates that are used all the time in this API. Typelists are used from his repo at [/LagMeester4000/TypeList](https://github.com/LagMeester4000/TypeList).
## Nintendo policies
Nintendo policies are strict, especially on fan-made content. If you are using this to hack roms, please be sure to NOT SUPPLY roms and if you're using this, please ensure that copyright policies are being kept. I am not responsible for any programs made with this, for it is only made to create new roms or use existing roms in the fair use policy.
### Legal issues
To prevent any legal issues, you can choose to distribute a 'patch' instead of a ROM. Distributing a ROM is illegal, it is copyrighted content and you are uploading it. Distributing a patch however, isn't illegal. This is because the patch doesn't contain any copyrighted info and just the data that was altered from the original ([Derivative work](https://en.wikipedia.org/wiki/Derivative_work)), thus falling under [fair use policy](https://en.wikipedia.org/wiki/Fair_use#Fair_use_and_reverse_engineering), as long as the modification is not commercial and the user hasn't agreed to the EULA. This patch contains no pieces of art, no pieces of code that have been untouched by the user of our program, therefore making it the user's property. The ROM is obtained through the player and modified by using the patch, however, whether or not the ROM is obtained legally is up to the player and the player must be sure that this process is legal in its country.
## Fair use
You can feel free to use this program any time you'd like, as long as you mention this repo.
