#include "archive.hpp"
using namespace nfs;

Archive::Archive() {}
Archive::~Archive() {
	clear();
}

Archive::Archive(NARC &narc, const String &debugName) {

	BTAF &btaf = narc.at<0>();
	Buffer btafb = narc.get<0>();

	if (btafb.size() != usz(btaf.files) * 8)
		EXCEPTION("Archive BTAF didn't match format");

	GMIF &gmif = narc.at<2>();
	Buffer gmifb = narc.get<2>();

	struct UncompComp {
		Buffer uncomp, comp;
		u32* sizePtr;
	};

	List<UncompComp> uncompComp(btaf.files);

	usz size = 0;
	for (u32 i = 0; i < btaf.files; ++i) {

		u32 *sizePtr = btafb.add<u32>(usz(i) * 8);

		u32 beg = sizePtr[0];
		u32 end = sizePtr[1];

		if (end < beg)
			EXCEPTION("Invalid file ptr");

		u8 *ptr = gmifb.add(beg);

		//We gotta apply decompression here, or our resource size will
		//be very messed up

		Buffer buf = Buffer(end - beg, ptr);
		Buffer comp = {};
		
		//Handle decompression

		if (CompressionHelper::getDecompressionAllocation(buf)) {

			buf = CompressionHelper::decompress(buf);

			if (buf.add() != ptr)
				comp = Buffer(end - beg, ptr);
		}

		//Handle when decompression flags are present but it doesn't allocate
		//(This means the data is already there, just with padding in front)

		else if(
			CompressionHelper::getCompressionType(
				buf.add(), buf.size()
			) != CompressionType::None
		)
			buf = CompressionHelper::decompress(buf);

		//

		uncompComp[i] = {
			buf, comp,
			sizePtr
		};

		size += ResourceHelper::getSize(
			ResourceHelper::getType(buf.add(), buf.size())
		);
	}

	buffer = Buffer::alloc(size);
	vec = List<ArchiveObject>(btaf.files);

	usz offset = 0;

	for (u32 i = 0; i < btaf.files; ++i) {

		const auto &ucCo = uncompComp[i];
		ArchiveObject &ao = vec[i];

		Buffer buf = ucCo.uncomp;

		//

		ao.filePtrs = ucCo.sizePtr;
		ao.buf = buf;
		ao.comp = ucCo.comp;

		ao.info.type = ResourceHelper::getType(buf.add(), buf.size());
		ao.info.magicNumber = ResourceHelper::getMagicNum(ao.info.type);
		ao.name = std::to_string(i) + "." + ResourceHelper::getName(buf.add(), buf.size(), ao.info.magicNumber);
		ao.position = buffer.add(offset);

		ResourceHelper::read(buf.add(), buf.size(), ao.position);

		offset += ao.info.size = ResourceHelper::getSize(ao.info.type);
	}
}

Archive::Archive(const Archive &other) { _copy(other); }
Archive::Archive(Archive &&other) { _move(std::move(other)); }

Archive &Archive::operator=(const Archive &other) { 
	clear();
	_copy(other); 
	return *this; 
}

Archive &Archive::operator=(Archive &&other) {
	clear();
	_move(std::move(other));
	return *this;
}

ArchiveObject &Archive::operator[](usz i) {

	if (i >= vec.size())
		EXCEPTION("Archive Couldn't find item");

	return vec[i];
}

const ArchiveObject &Archive::operator[](usz i) const {

	if (i >= vec.size())
		EXCEPTION("Archive Couldn't find item");

	return vec[i];
}

void Archive::clear() {

	for (ArchiveObject &obj : vec)
		if(obj.compression())
			obj.buf.dealloc();

	buffer.dealloc();
	vec.clear();
}

void Archive::_move(Archive &&other) {

	buffer = other.buffer;
	vec = std::move(other.vec);

	other.vec.clear();
	other.buffer = Buffer();
}

void Archive::_copy(const Archive &other) {

	buffer = Buffer::copy(other.buffer);
	vec = other.vec;

	for (auto &a : vec) {

		u8 *&pos = a.position;
		pos = buffer.add(pos - other.buffer.add());

		if (a.compression())
			a.buf = a.buf.clone();
	}
}