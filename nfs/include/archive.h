#pragma once
#include "resourcehelper.h"
#include <exception>

namespace nfs {

	struct ArchiveObject {
		ResourceInfo info;
		std::string name;
		u8 *position;
	};

	class Archive {

	public:

		Archive();
		~Archive();
		Archive(NARC &narc);
		Archive(const Archive &other);
		Archive(Archive &&other);

		Archive &operator=(const Archive &other);

		template<typename T>
		T &at(u32 i);

		template<typename T>
		bool isType(u32 i) const;

		u32 size() const;

		ArchiveObject &operator[](u32 i);

		std::vector<ArchiveObject> &getObjects();
		Buffer getBuffer();

		ArchiveObject *data();

		void clear();

	protected:

		void copy(const Archive &other);

		Buffer buffer;
		std::vector<ArchiveObject> vec;
	};


	template<typename T>
	T &Archive::at(u32 i) {

		ArchiveObject &ao = (*this)[i];

		if (ao.info.magicNumber != ResourceHelper::getMagicNumber(T{}))
			throw std::exception("Archive Couldn't cast; variables aren't of same type");

		return *(T*)ao.position;
	}


	template<typename T>
	bool Archive::isType(u32 i) const {
		ArchiveObject &ao = *const_cast<ArchiveObject*>(&(*this)[i]);
		return ao.info.magicNumber == ResourceHelper::getMagicNumber(T{});
	}

}