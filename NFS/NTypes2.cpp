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

	return resources[i]->header.magicNumber;
}

std::string NArchive::getTypeName(u32 i) const {
	u32 t = getType(i);
	std::string typeName = std::string((char*)&t, 4);
	return typeName;
}

u32 NArchive::size() const { return resources.size(); }
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