#include "file_system_object.hpp"
#include "compression_helper.hpp"

namespace nfs {

	void FileSystemObject::commit() {

		if (!isCompressed())
			return;

		Buffer largerComp = Buffer(reservedSize, compressed.add());

		Buffer res = CompressionHelper::compress(buf, largerComp);

		if (filePtr1) {
			*filePtr1 = u32(res.size());
			return;
		}

		filePtrs[1] = u32(filePtrs[0] + res.size());
	}

}