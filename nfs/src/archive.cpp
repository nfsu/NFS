#include "archive.h"
using namespace nfs;

Archive::Archive() {}
Archive::~Archive() {
	buffer.dealloc();
}

Archive::Archive(NARC &narc) {

	BTAF &btaf = narc.at<0>();
	Buffer btafb = narc.get<0>();

	if (btafb.size != btaf.files * 8)
		EXCEPTION("Archive BTAF didn't match format");

	Buffer gmifb = narc.get<2>();

	u32 size = 0;
	for (u32 i = 0; i < btaf.files; ++i) {

		const u32 &beg = *(u32*)(btafb.ptr + i * 8U);

		u32 type = ResourceHelper::getType(gmifb.ptr + beg);
		size += (u32)ResourceHelper::getSize(type);
	}

	buffer = Buffer::alloc(size);
	vec = std::vector<ArchiveObject>(btaf.files);

	u32 offset = 0;

	for (u32 i = 0; i < btaf.files; ++i) {

		const u32 &beg = *(u32*)(btafb.ptr + i * 8U);
		const u32 &end = *(u32*)(btafb.ptr + i * 8U + 4U);
		const u32 len = end - beg;

		ArchiveObject &ao = vec[i];
		u8 *rptr = gmifb.ptr + beg;

		ao.info.type = ResourceHelper::getType(rptr);
		ao.info.magicNumber = ResourceHelper::getMagicNum(ao.info.type);
		ao.name = std::to_string(i) + "." + ResourceHelper::getName(rptr, ao.info.magicNumber);
		ao.position = buffer.ptr + offset;

		ResourceHelper::read(rptr, len, ao.position);

		offset += (u32)(ao.info.size = ResourceHelper::getSize(ao.info.type));
	}

}

Archive::Archive(const Archive &other) { copy(other); }

Archive::Archive(Archive &&other) {

	buffer = other.buffer;
	vec = std::move(other.vec);

	other.vec.clear();
	other.buffer = Buffer();
}

Archive &Archive::operator=(const Archive &other) { copy(other); return *this; }

u32 Archive::size() const { return (u32)vec.size(); }

ArchiveObject &Archive::operator[](u32 i) {

	if (i >= vec.size())
		EXCEPTION("Archive Couldn't find item");

	return vec[i];
}

const ArchiveObject &Archive::operator[](u32 i) const {

	if (i >= vec.size())
		EXCEPTION("Archive Couldn't find item");

	return vec[i];
}

std::vector<ArchiveObject> &Archive::getObjects() { return vec; }
Buffer Archive::getBuffer() { return buffer; }

ArchiveObject *Archive::data() { return vec.data(); }

void Archive::clear() {
	buffer.dealloc();
	vec.clear();
}

void Archive::copy(const Archive &other) {

	buffer = Buffer::copy(other.buffer);
	vec = other.vec;

	for (auto &a : vec) {
		u8 *&pos = a.position;
		pos = buffer.ptr + (pos - other.buffer.ptr);
	}
}