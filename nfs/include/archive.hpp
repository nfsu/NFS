#pragma once
#include "resource_helper.hpp"

namespace nfs {

	struct ArchiveObject {

		ResourceInfo info;

		String name;
		u8 *position;

		Buffer buf, comp;

		u32 *filePtrs;

		inline bool compression() const { return comp.size(); }
	};

	class Archive {

	public:

		Archive();
		virtual ~Archive();
		Archive(NARC &narc, const String &debugName = {});
		Archive(const Archive &other);
		Archive(Archive &&other);

		Archive &operator=(const Archive &other);
		Archive &operator=(Archive &&other);

		template<typename T>
		T &at(usz i);

		template<typename T>
		T &get(ArchiveObject &ao);

		template<typename T>
		bool isType(u32 i) const;

		inline usz size() const { return vec.size(); }

		ArchiveObject &operator[](usz i);
		const ArchiveObject &operator[](usz i) const;

		List<ArchiveObject> &getObjects() { return vec; }
		Buffer getBuffer() { return buffer; }

		ArchiveObject *data() { return vec.data(); }

		void clear();

	protected:

		void _copy(const Archive &other);
		void _move(Archive &&other);

		Buffer buffer;
		List<ArchiveObject> vec;
	};


	template<typename T>
	T &Archive::at(usz i) {
		return get<T>((*this)[i]);
	}

	template<typename T>
	T &Archive::get(ArchiveObject &ao) {

		if (ao.info.magicNumber != ResourceHelper::getMagicNumber(T{}))
			EXCEPTION("Archive Couldn't cast; variables aren't of same type");

		return *(T*) ao.position;
	}


	template<typename T>
	bool Archive::isType(u32 i) const {
		const ArchiveObject &ao = (*this)[i];
		return ao.info.magicNumber == ResourceHelper::getMagicNumber(T{});
	}

}