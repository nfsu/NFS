# Nintendo File System Utils

NFSU is the visual interface to the NFS project. Its UI allows you to edit and view contents of an NDS file. The tool was originally only created to view resources, but after NFS got changed to allow runtime modifications to the ROM, it got an upgrade to allow for changing resources at runtime.

The system currently doesn't support viewing and modifying the source code, only resources. It probably won't support it, though pull requests and contributions to support this are very much appreciated.

Expanding roms is also not on the roadmap, even though emulated roms could support up to 4 GiB of files. The NDS itself won't allow more than 256 MiB for the ROM, though we might provide a free space locator at some point (to allow adding more resources). Expanding NDS files beyond their 256 MiB limit will also cause the patcher to malfunction, as it requires two identical sized files. 