#pragma once

#include "Generic.h"

namespace nfs {

	template<class T> struct ArchieveObject {
		u32 offset, id;
		std::string type;
		Buffer contents;
		T value;
	};

	template<class T> class Archieve {

	public:

		Archieve(std::vector<ArchieveObject<T>> _archieves, u32 _size) : archieves(_archieves), bsize(_size) {}
		Archieve(): archieves(std::vector<ArchieveObject<T>>()), bsize(0) {}

		u32 getSize() { return archieves.size(); }
		u32 getFileSize() { return bsize; }

		const ArchieveObject<T> &operator[](u32 i) { return archieves[i]; }

	private:

		std::vector<ArchieveObject<T>> archieves;
		u32 bsize;

	};
}