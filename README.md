![NFS logo](https://i.imgur.com/uabtvGW.png)
<br>
Nintendo File System
-------

NFS(U) stands for Nintendo File System (Utils) and is designed to read and interpret nds files such as NARC, NCLR, NCGR, etc.  
# But why?
Some time ago, I loved modifying existing roms for my personal use, but tools were/are either extremely lacking (both in use and looks) or they frequently crashed and were a pain to use. I finally decided to look into Nintendo's file system and how things could be written / read. Thanks to template magician [LagMeester4000](https://github.com/LagMeester4000), I 'simplified' this system using C++ templates, allowing you to add custom formats within a few seconds. The reading/writing/converting is very quick and can be easily used in existing applications.
# How to install
The newest version of NFS uses cmake to generate the build projects for you, you can simply run 'reload.bat' and it will generate the files for you. With this, you can add new projects and depend on NFS/NFSU. Simply run the command;
```bat
addproject nfse nfs
```
The example above will automatically add the 'nfse' project to the CMakeLists and will create a basic header and source file for you. It then reloads the project, so your project is added. If you want to depend on nfsu instead, you change 'nfs' to 'nfsu'. If you want to make one yourself, you simply create a CMakeLists.child.txt file in the root of your project and then you put in everything an inherrited CMakeLists.txt would have, where <PROJECT_NAME> is a macro for the project name put in in addproject.
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
Texture2D tilemap(ncgr, nclr);
Texture2D map(nscr, ncgr, nclr);
```
'Texture2D' is a simple 2D texture (could be any format; but in NDS it's BGR555, 4 bit or 8 bit most of the time). If you use any texture conversions (such as tilemap and map), it will allocate memory it copies around; so use std::move whenever you pass this texture around to prevent copying.
### FileSystem's parent
FileSystem is a unique object, it has a folder structure. But, it still remains a list of resources. This is why it uses NArchive as its parent. It stores both resources and file information. This means that you can use the archive's functions too, but those can't be used in combination with file names.
### Archives
Archives kind of work like a FileSystem, only you don't have named files & folders, you just have files with indices (and extension). You can literally loop or iterate through the ArchiveObject's and read those values. 
#### Flaws in resource reading
Resource reading interprets the data of the ROM into a struct; which means that some data can't be interpreted when you try loading them. This will create an object named 'NBUO' (Buffer Unknown Object) and it is just a buffer, it has a pointer of what data it needs to parse and the length of the data. You can check for common types and try to parse them yourselves, when necessary.
```cpp
try {
	NBUO &nbuo = arch.at<NBUO>(i);
	printf("Undefined object at %u (%p) with size %u\n", i, nbuo.ptr, nbuo.size);
}
catch (std::exception e) {}
```
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
typedef GenericResource<0x4352414E, BTAF, BTNF, GMIF> NARC;
```
A NARC is defined as a resource containing a BTAF, BTNF and GMIF. The GMIF contains the archieve buffer, BTNF contains the offsets and buffer lengths.
The first value is its magic number; it indicates how it can recognize a NARC in the ROM.
Afterwards, they also need to be inserted into the ResourceTypes array, so the auto parser for the NArchive / FileSystem can recognize them.
```cpp
typedef CompileTimeList<NCLR, NCGR, NSCR, NARC, NBUO> ResourceTypes;
```
### Writing a resource type
The reason this API is so fast is it rarely mallocs; all resources are structs or use the ROM buffer. This means that the rom is directly affected if you write to a GenericResource's buffer (or convert it to things like textures). The following is how you modify a 64x64 image:
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
		tex.write(i, j, 0x1 + r * 0xE);
	}
```
'write' doesn't just set the data in the texture; it also checks what kind of texture is used. If you are using a BGR555 texture, you input RGBA8 but it has to convert it first. 'read' does the same; except it changes the output you receive.
###  Writing textures
Textures aren't that easy in NFS; palettes are always used and sometimes, they even use tilemaps. This means that fetching the data directly won't return an RGBA8 color, but rather an index to a palette or tile. If you want to output the actual image, you can create a new image that will be converted from the ROM's image.
```cpp
Texture2D tex2(ncgr, nclr);
tex2.write("Final0.png");
```
### Writing image filters
If you'd want to add a new cpu 'shader', I've created a helpful function, which can be used as the following:
```cpp
u32 convertFromNCGR_func(Texture2D t, u16 i, u16 j, Texture2D tilemap, Texture2D palette) { 
	u32 sample = tilemap.fetch(i, j);
	u32 x = sample & 0xF;
	u32 y = (sample & 0xF0) >> 4;
	return palette.read(x, y);
}

Texture2D convertFromNCGR(NCGR &ncgr, NCLR &nclr) {
	Texture2D ncgrt = Texture2D(ncgr);
	return fromShader(convertFromNCGR_func, (ncgrt.getWidth(), (ncgrt.getHeight(), ncgrt, Texture2D(nclr));
}
```
This function is called 'fromShader', it will create a new RGBA8 texture and for every pixel, it will run the function supplied. The function needs to return a u32 and take a Texture2D tex, u16 x, u16 y and the arguments you put in yourself. Remember to delete it when you don't need it anymore though.
## Supported file formats
For supported file formats, check out the docs folder; this contains a description of every supported resource and their magicNumber/type id.
## Special thanks
Thanks to /LagMeester4000 for creating magic templates that are used all the time in this API. Typelists are used from his repo at [/LagMeester4000/TypeList](https://github.com/LagMeester4000/TypeList).
## Nintendo policies
Nintendo policies are strict, especially on fan-made content. If you are using this to hack roms, please be sure to NOT SUPPLY roms or ROM files and if you're using this, please ensure that copyright policies are being kept. I am not responsible for any programs made with this, for it is only made to create new roms or use existing roms in the fair use policy.
### Legal issues
To prevent any legal issues, you can choose to distribute a 'patch' instead of a ROM. Distributing a ROM is illegal, it is copyrighted content and you are uploading it. Distributing a patch however, isn't illegal. This is because the patch doesn't contain any copyrighted info and just the data that was altered from the original ([Derivative work](https://en.wikipedia.org/wiki/Derivative_work)), thus falling under [fair use policy](https://en.wikipedia.org/wiki/Fair_use#Fair_use_and_reverse_engineering), as long as the modification is not commercial and the user hasn't agreed to the EULA. This patch contains no pieces of art, no pieces of code that have been untouched by the user of our program, therefore making it the user's property. The ROM is obtained through the player and modified by using the patch, however, whether or not the ROM is obtained legally is up to the player and the player must be sure that this process is legal in its country.
## Fair use
You can feel free to use this program any time you'd like, as long as you mention this repo.
