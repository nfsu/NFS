# NFS ![Logo](https://i.imgur.com/bBy2kS9.png)
Nintendo File System
NFS(U) stands for Nintendo File System (Utils) and is designed to read and interpret nds files such as NARC, NCLR, NCGR, etc.  
![Logo](https://i.imgur.com/PWpc5ZX.png)
## Why NFS?
As a kid I loved modifying existing roms for my personal use, but tools were/are either extremely lacking (both in use and looks) or they frequently crashed and were a pain to use. I finally decided to look into Nintendo's file system and how things could be written / read. Thanks to template magician /LagMeester4000, I 'simplified' this system using C++ templates, allowing you to add custom formats within a few seconds. The reading/writing/converting is very quick and can be easily used in existing applications.
## How to use
Down below you can see a simply use of the NFS API.
### Reading raw ROM data
```cpp
	Buffer buf = readFile("ROM.nds");
	try {
		NDS nds = NType::readNDS(buf);
	} catch (std::exception e) {
		printf("%s\n", e.what());
	}
	deleteBuffer(&buf);
```
If you've acquired your ROM, you have to load it into memory, so the program can read and modify (a copy) of it. This buffer can be deleted afterwards (and written to a folder, to save all changes made).  
NDS is a format that seperates the header from the contents for a NDS file; it stores the important information in a struct and the other stuff in a buffer.
### Converting the ROM into a FileSystem
A ROM is like a ZIP; except it is used for storing game data (models, images, sounds, palettes, maps, binary data, text, code and more). This means that it stores the file names into the ROM; which we can use to extract the files we need and where they are.
```cpp
		FileSystem files;
		NType::convert(nds, &files);
```
This will put the file data into the files variable, which you can then loop through and use.
```cpp
		for (auto iter = files.begin(); iter != files.end(); ++iter) {

			FileSystemObject fileSysObj = *iter;
			//More code...
		}
```
### Checks for fso's
Fso stands for 'FileSystemObject' and it is what I call folders and files; this means that fileSysObj isn't always a file, it could also be a folder. To distinguish them, you can use the following functions:
- isFile
- isFolder
- isRoot
- hasParent
- getMagicNumber(std::string &type, u32 &magicNum)
'getMagicNumber' will try to figure out the type name for a file and will also try to return a valid 'magicNumber' (an identifier for any type of class).  
So, before converting a file, make sure it is actually a file.
### Obtaining the resource of the file
```cpp
			if (fileSysObj.isFile()) {

				std::string name;						//Typename of the file
				u32 magicNumber;						//MagicNumber / indication
				bool valid = fileSysObj.getMagicNumber(name, magicNumber);	//Whether or not the file is supported

				try {
					const NBUO &nbuo = files.getResource<NBUO>(fileSysObj);
					printf("Object: %s (%s)\n", val.path.c_str(), name.c_str());
					//Cast it to a 'Buffer Unknown Object', this scope only runs when the file is not supported
				}
				catch (std::exception e) {
					printf("Supported object: %s (%s)\n", val.path.c_str(), name.c_str());	
					//If it's not an unknown object; it's supported
				}
			}
			else {
				//It's not a file, but a folder or root folder
			}
```
A resource can be anything; a buffer, a file, an image, a palette, a map, etc. It is anything that is stored in the ROM's file system. This resource has to be converted, just like the NDS file, before you can use the contents.
### Traversing a folder
'Traversing' is what I call searching through an entire folder; so traverse through /someFolder/something would give all of the folders and files in the directory and inside those folders. While the square brackets (or array operator) are used for obtaining folders/files directly inside a folder:
```cpp
	std::vector<const FileSystemObject*> fsos = files.traverseFolder(files["fielddata"]);
	for (u32 i = 0; i < fsos.size(); ++i)
		printf("%s\n", fsos[i]->path.c_str());
```
`files["x"]` will get the fso at the position of x; that FSO can then be used to trace all files or get the files inside of that folder (if it is a folder). You could also traverse root; by using "/" as the folder, or simply by using `files[fso]`.
```cpp
	std::vector<const FileSystemObject*> fsos = files[files["/"]];
	for (u32 i = 0; i < fsos.size(); ++i)
		printf("%s\n", fsos[i]->path.c_str());
```
### Reading resources
You can automatically read resources from an archive/file system by simply converting an NDS to a file system or a NARC to a NArchive, like so:
```cpp
	//Archives
	NArchive arch;
	NType::convert(narc, &arch);
	//FileSystems
	FileSystem fs;
	NType::convert(nds, &fs);
```
NArchive contains a buffer with all of the types, while FileSystem also contains the names and relations of the files. These types can be converted again to get things like Texture2D, NArchive, etc.
```cpp
	Texture2D tex;
	NType::convert(nclr, &tex);
```
### FileSystem's parent
FileSystem is a unique object, it has a folder structure. But, it still remains a list of resources. This is why it uses NArchive as its parent. It stores both resources and file information. This means that you can use the archive's functions too, but those can't be used in combination with file names.
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
## Special thanks
Thanks to /LagMeester4000 for creating magic templates that are used all the time in this API. Typelists are used from his repo at [/LagMeester4000/TypeList](https://github.com/LagMeester4000/TypeList).
## Nintendo policies
Nintendo policies are strict, especially on fan-made content. If you are using this to hack roms, please be sure to NOT SUPPLY roms and if you're using this, please ensure that copyright policies are being kept. I am not responsible for any programs made with this, for it is only made to create new roms or use existing roms in the fair use policy.
## Fair use
You can feel free to use this program any time you'd like, as long as you mention this repo.
