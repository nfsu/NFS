#include "NTypes2.h"
using namespace nfs;

NArchieve::NArchieve(std::vector<GenericResourceBase*> _resources, Buffer _buf) : resources(_resources), buf(_buf) {}
NArchieve::NArchieve() {}
NArchieve::~NArchieve() { deleteBuffer(&buf); }

NArchieve::NArchieve(const NArchieve &other) {
	copy(other);
}

NArchieve &NArchieve::operator=(const NArchieve &other) {
	copy(other);
	return *this;
}

u32 NArchieve::getType(u32 i) const {
	if (i >= resources.size())
		throw(std::exception("Out of bounds"));

	return resources[i]->header.magicNumber;
}

std::string NArchieve::getTypeName(u32 i) const {
	u32 t = getType(i);
	std::string typeName = std::string((char*)&t, 4);
	return typeName;
}

u32 NArchieve::size() const { return resources.size(); }
u32 NArchieve::bufferSize() const { return buf.size; }

void NArchieve::copy(const NArchieve &other) {

	if (other.buf.data != NULL)
		buf = newBuffer3(other.buf.data, other.buf.size);
	else
		buf = { NULL, 0 };

	resources = other.resources;

	for (u32 i = 0; i < other.size(); ++i) {
		resources[i] = (GenericResourceBase*)(((u8*)other.resources[i]) - other.buf.data + buf.data);
		if (resources[i]->header.magicNumber == 0) {
			((NBUO*)resources[i])->contents.front.name = ((NBUO*)other.resources[i])->contents.front.name;
		}
	}
}

NDS NType::readNDS(Buffer buf) {
	NDS nds;
	memcpy((u8*)(&nds) + GenericSection_begin, buf.data, sizeof(nds) - GenericSection_begin);
	nds.data = offset(buf, nds.romHeaderSize);
	return nds;
}