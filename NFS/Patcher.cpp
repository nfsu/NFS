#include "Patcher.h"
#include "Timer.h"
#include <future>
using namespace nfs;

bool Patcher::patch(std::string original, std::string patch, std::string out) {
	return false;
}

Buffer Patcher::patch(Buffer original, Buffer patch) {

	return { nullptr, 0 };
}

bool Patcher::compare(Buffer a, Buffer b, std::vector<Buffer> &result, u32 &compares) {

	if (a.size != b.size) return false;

	u32 division = a.size / 4;
	u32 remainder = a.size % 4;

	bool res = true;
	for (u32 i = 0; i < 4; ++i) {
		u32 size = division + (i == 3 ? remainder : 0);

		Buffer iA = { a.data + division * i, size };
		Buffer iB = { b.data + division * i, size };

		++compares;
		if (memcmp(iA.data, iB.data, iA.size) == 0) continue;

		res = false;

		if (size <= 1024U) {
			result.push_back(iB);
			continue;
		} else
			compare(iA, iB, result, compares);
	}

	return res;
}

Buffer Patcher::writePatch(Buffer original, Buffer modified) {

	oi::Timer t;

	static const char magicNum[4] = { 'N', 'F', 'S', 'P' };

	NFSP_Header header;
	memcpy(&header, magicNum, 4);
	header.padding = 0;
	header.size = modified.size;

	u32 end;
	std::vector<NFSP_Block> blocks;

	if (modified.size > original.size) {
		blocks.push_back({ original.size, modified.size - original.size, offset(modified, original.size) });
		end = original.size;
	}
	else
		end = modified.size;

	u32 thrs;
	std::vector<std::future<std::vector<Buffer>>> futures(thrs = std::thread::hardware_concurrency());
	u32 subdivided = end / thrs;
	u32 remainder = end % thrs;

	for (u32 i = 0; i < thrs; ++i) {
		u32 val = subdivided + (i == thrs - 1 ? remainder : 0);
		u32 in = i;
		futures[i] = std::move(std::async([&, in]() -> std::vector<Buffer> {

			std::vector<Buffer> differences;
			u32 compares = 0;

			compare({ original.data + in * subdivided, val }, { modified.data + in * subdivided, val }, differences, compares);

			//printf("Compared %u buffers\n", compares);
			return differences;
		}));
	}

	u32 finalSize = sizeof(NFSP_Header);

	for (u32 i = 0; i < thrs; ++i) {
		std::vector<Buffer> bufs = std::move(futures[i].get());

		if (bufs.size() != 0) {
			u32 off = blocks.size();
			blocks.resize(off + bufs.size());

			for (u32 j = 0; j < bufs.size(); ++j) {
				Buffer &buf = bufs[j];
				blocks[off + j] = { (u32)(buf.data - modified.data), buf.size, buf };
				finalSize += 8 + buf.size;
			}
		}
	}

	header.blocks = blocks.size();

	Buffer result = newBuffer1(finalSize);
	memcpy(result.data, &header, sizeof(header));

	Buffer offb = offset(result, sizeof(header));

	for (u32 i = 0; i < blocks.size(); ++i) {

		NFSP_Block *blc = &blocks[i];

		memcpy(offb.data, blc, 8);
		memcpy(offb.data + 8, blc->buf.data, blc->length);
		offb = offset(offb, 8 + blc->length);
	}

	t.stop();
	printf("Completed writing patch:\n");
	t.print();

	return result;
}

bool Patcher::writePatch(std::string original, std::string modified, std::string patch) {
	Buffer og = readFile(original);
	Buffer mod = readFile(modified);

	if (og.size == 0) {
		if (mod.size != 0)
			deleteBuffer(&mod);
		return false;
	}

	if (mod.size == 0) {
		if (og.size != 0)
			deleteBuffer(&og);
		return false;
	}

	Buffer res = writePatch(og, mod);
	if (res.size == 0) {
		deleteBuffer(&mod);
		deleteBuffer(&og);
		return false;
	}

	bool b = writeBuffer(res, patch);
	deleteBuffer(&res);
	return b;
}