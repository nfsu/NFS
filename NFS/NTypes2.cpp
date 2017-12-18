#include "NTypes2.h"
using namespace nfs;

NArchive::NArchive(std::vector<GenericResourceBase*> _resources, Buffer _buf) : resources(_resources), buf(_buf) {}
NArchive::NArchive() {}
NArchive::~NArchive() { deleteBuffer(&buf); }

NArchive::NArchive(const NArchive &other) {
	copy(other);
}

NArchive &NArchive::operator=(const NArchive &other) {
	copy(other);
	return *this;
}

u32 NArchive::getType(u32 i) const {
	if (i >= resources.size())
		throw(std::exception("Out of bounds"));

	u32 mn = resources[i]->header.magicNumber;
	bool isValid = false;
	runArchiveFunction<NType::IsValidType>(mn, ArchiveTypes(), &isValid);

	return mn;
}

bool NArchive::copyResource(u32 id, u8 *where, u32 size) {
	if(id >= resources.size()) return false;

	memcpy(where, resources[id], size);
	return true;
}

std::string NArchive::getTypeName(u32 i) const {
	u32 t = getType(i);
	std::string typeName = std::string((char*)&t, 4);
	std::reverse(typeName.begin(), typeName.end());

	for (u32 i = 0; i < 4; ++i) {
		char &c = typeName[i];
		if (!(c >= '0' && c <= '9') && !(c >= 'A' && c <= 'Z') && !(c >= 'a' && c <= 'z'))
			c = '?';
	}

	return typeName;
}

u32 NArchive::size() const { return (u32)resources.size(); }
u32 NArchive::bufferSize() const { return buf.size; }

void NArchive::copy(const NArchive &other) {

	if (other.buf.data != NULL)
		buf = newBuffer3(other.buf.data, other.buf.size);
	else
		buf = { NULL, 0 };

	resources = other.resources;

	for (u32 i = 0; i < other.size(); ++i) 
		resources[i] = (GenericResourceBase*)(((u8*)other.resources[i]) - other.buf.data + buf.data);
}

NDS NType::readNDS(Buffer buf) {
	NDS nds;
	memcpy((u8*)(&nds) + GenericSection_begin, buf.data, sizeof(nds) - GenericSection_begin);
	nds.data = offset(buf, nds.romHeaderSize);
	return nds;
}

bool NType::convert(NARC source, NArchive *archieve) {

	BTAF &btaf = source.contents.front;
	u32 files = btaf.files;

	if (btaf.data.size != btaf.files * 8 || files == 0) {
		printf("Couldn't convert NARC to Archieve; invalid file info\n");
		return false;
	}

	u32 bufferSize = 0;

	for (u32 i = 0; i < files; ++i) {

		u32 off = getUInt(offset(btaf.data, i * 8));
		u32 size = getUInt(offset(btaf.data, i * 8 + 4)) - off;
		u8 *data = source.contents.back.back.front.data.data + off;

		u32 magicNumber = *(u32*)data;

		u32 offInBuffer = bufferSize;

		try {
			runArchiveFunction<GenericResourceSize>(magicNumber, ArchiveTypes(), &bufferSize);
		}
		catch (std::exception e) {}

		if (offInBuffer == bufferSize)
			bufferSize += sizeof(NBUO);
	}

	u32 currOff = 0;

	u32 someOtherInt = *(u32*)(source.contents.back.front.data.data + 4);

	Buffer buf = newBuffer1(bufferSize);
	std::vector<GenericResourceBase*> resources(files);

	for (u32 i = 0; i < files; ++i) {

		u32 off = getUInt(offset(btaf.data, i * 8));
		u32 size = getUInt(offset(btaf.data, i * 8 + 4)) - off;
		u8 *data = source.contents.back.back.front.data.data + off;

		u32 magicNumber = *(u32*)data, oMagicNumber = magicNumber;

		u32 offInBuffer = currOff;

		try {
			runArchiveFunction<GenericResourceSize>(magicNumber, ArchiveTypes(), &currOff);
		}
		catch (std::exception e) {}

		u8 *loc = buf.data + offInBuffer;
		Buffer b = { data, size };

		if (currOff == offInBuffer) {
			currOff += sizeof(NBUO);
			magicNumber = 0;
		}

		try {
			runArchiveFunction<NFactory>(magicNumber, ArchiveTypes(), (void*)loc, b);
		}
		catch (std::exception e) {}

		resources[i] = (GenericResourceBase*)loc;
	}

	*archieve = NArchive(resources, buf);

	return true;
}

bool NType::convert(NCLR source, Texture2D *tex) {
	tex->width = source.contents.front.c_colors;
	tex->size = source.contents.front.dataSize;
	tex->stride = 2;
	tex->tt = BGR5;
	tex->height = tex->size / tex->stride / tex->width;
	tex->data = source.contents.front.data.data;
	return true;
}


bool NType::convert(NCGR source, Texture2D *tex) {
	bool fourBit = source.contents.front.tileDepth == BD_FOUR;

	tex->width = source.contents.front.tileWidth * 8;
	tex->height = source.contents.front.tileHeight * 8;

	//Unknown parts of NCGR;
	//width & height: 0xFFFF
	//padding; 1048592, 24
	//16, 24

	if (tex->width / 8 == 0xFFFF || tex->height / 8 == 0xFFFF) {		//Special flag?
		tex->width = (u32)sqrt(source.contents.front.tileDataSize * (fourBit ? 2 : 1));
		tex->height = tex->width;
	}

	tex->size = tex->width * tex->height / (fourBit ? 2 : 1);
	tex->tt = fourBit ? TILED8_B4 : TILED8;
	tex->stride = 1;
	tex->data = source.contents.front.data.data;
	return true;
}


bool NType::convert(NSCR source, Texture2D *tex) {
	tex->width = source.contents.front.screenWidth / 8;
	tex->height = source.contents.front.screenHeight / 8;
	tex->size = tex->width * tex->height * 2;
	tex->tt = NORMAL;
	tex->stride = 2;
	tex->data = source.contents.front.data.data;
	return true;
}