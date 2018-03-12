# Resources
File extension | Type Id | File name | Magic number | Data type | Conversion type | isResource
--- | --- | --- | --- | --- | --- | ---
NCLR | 0 | CoLor Resource | 0x4E434C52 | Palette | Texture2D | yes
NCGR | 1 | Character Graphics Resource | 0x4E434752 | Tilemap / image | Texture2D | yes
NSCR | 2 | SCreen Resource | 0x4E534352 | Map | Texture2D | yes
NARC | 3 | Archive | 0x4352414E | Archive | Archive | yes
NBUO | 4 | - | 0x4E42554F | Buffer | - | yes
NDS | - | Dual Screen | - | File system and code | FileSystem | no
<br>
If your format is not on here, then you probably can't read the file without effort. You probably have to reverse engineer and/or implement it yourself.
