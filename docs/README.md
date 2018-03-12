# Resources
The following resources are implemented in NFS:
Type id | File extension | File name | Magic number | Data type | Conversion type | isResource
--- | --- | --- | --- | --- | ---
0 | NCLR | CoLor Resource | 0x4E434C52 | Palette | Texture2D | yes
1 | NCGR | Character Graphics Resource | 0x4E434752 | Tilemap / image | Texture2D / PaletteTexture2D | yes
2 | NSCR | SCreen Resource | 0x4E534352 | Map | Texture2D / TiledTexture2D | yes
3 | NARC | Archive | 0x4352414E | Archive | NArchive | yes
- | NDS | Dual Screen | - | File system and code | FileSystem | no
If your format is not on here, then you probably can't read the file without effort. You probably have to reverse engineer and/or implement it yourself.