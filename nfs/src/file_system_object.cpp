#include "file_system_object.hpp"
#include "compression_helper.hpp"

namespace nfs {

	void FileSystemObject::commit() {

		if (!isCompressed())
			return;

		Buffer largerComp = Buffer(reservedSize, compressed.add());

		Buffer res = CompressionHelper::compress(buf, largerComp);

		filePtrs[1] = u32(filePtrs[0] + res.size());
	}

}