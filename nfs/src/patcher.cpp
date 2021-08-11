#include "patcher.hpp"
#include "timer.hpp"
#include <thread>
#include <future>
#include "bitset.hpp"
#include <math.h>
#include <algorithm>
using namespace nfs;

bool Patcher::patch(const String &original, const String &path, const String &out) {

	Buffer og = Buffer::readFile(original);
	Buffer ptc = Buffer::readFile(path);

	if (!og.size()) {

		if (ptc.size())
			ptc.dealloc();

		return false;
	}

	if (!ptc.size()) {

		if (og.size())
			og.dealloc();

		return false;
	}

	Buffer output = patch(og, ptc);

	if (!output.size()) {
		og.dealloc();
		ptc.dealloc();
		return false;
	}

	bool b = output.writeFile(out);
	output.dealloc();
	og.dealloc();
	ptc.dealloc();
	return b;
}

Buffer Patcher::patch(Buffer original, Buffer patch) {

	oi::Timer t;

	NFSP_Header head = patch.consume<NFSP_Header>();

	static const c8 magicNum[4] = { 'N', 'F', 'S', 'P' };

	if (std::memcmp(&head, magicNum, 4)) {
		printf("Couldn't patch file! Patch was invalid\n");
		return { };
	}

	Buffer output = Buffer::alloc(head.size);
	usz len = original.size() > head.size ? head.size : original.size();
	std::memcpy(output.begin(), original.begin(), len);

	for (u32 i = 0; i < head.blocks; ++i) {
		NFSP_BlockHeader block = patch.consume<NFSP_BlockHeader>();
		std::memcpy(output.begin() + block.offset, patch.splitConsume(block.length).begin(), block.length);
	}

	t.stop();
	printf("Finished patching a buffer:\n");
	t.print();

	return output;
}

bool Patcher::compare(Buffer a, Buffer b, List<Buffer> &result, u32 &compares) {

	if (a.size() != b.size()) 
		return false;

	usz division = a.size() >> 2;
	usz remainder = a.size() & 3;

	bool res = true;

	for (u32 i = 0; i < 4; ++i) {

		usz size = division + (i == 3 ? remainder : 0);

		Buffer iA = { size, a.add(division * i) };
		Buffer iB = { size, b.add(division * i) };

		++compares;

		if (std::memcmp(iA.add(), iB.add(), iA.size()) == 0) 
			continue;

		res = false;

		if (size <= 1024U) {

			Buffer buf = {  };
			bool match = true;

			for (u32 j = 0; j < size; ++j) {

				bool curr = iA.at(j) == iB.at(j);

				if (match && !curr) 				//Start of buffer
					buf = { size - j, iB.add(j) };
				else if(curr && !match){			//End of buffer
					result.push_back({ usz(iB.add(j) - buf.add()), buf.add() });
					buf = {  };
				}

				match = curr;
			}

			if (!match)							//End of buffer
				result.push_back({ usz(iB.add(size) - buf.add()), buf.add() });
			
			continue;
		} 

		else compare(iA, iB, result, compares);
	}

	return res;
}

Buffer Patcher::writePatch(Buffer original, Buffer modified) {

	oi::Timer t;

	static const char magicNum[4] = { 'N', 'F', 'S', 'P' };

	NFSP_Header header;
	std::memcpy(&header, magicNum, 4);
	header.size = modified.size();

	u64 end;
	List<NFSP_Block> blocks;

	if (modified.size() > original.size()) {

		blocks.push_back(NFSP_Block{ 
			original.size(), modified.size() - original.size(),
			modified.offset(original.size()) 
		});

		end = original.size();
	}

	else end = modified.size();

	u32 thrs = std::thread::hardware_concurrency();
	List<std::future<List<Buffer>>> futures(thrs);
	u64 subdivided = end / thrs;
	u64 remainder = end % thrs;

	for (u32 i = 0; i < thrs; ++i) {

		u64 val = subdivided + (i == thrs - 1 ? remainder : 0);
		u32 in = i;

		futures[i] = std::move(std::async([&, in]() -> List<Buffer> {

			List<Buffer> differences;
			u32 compares = 0;

			compare(
				{ val, original.add(in * subdivided) }, 
				{ val, modified.add(in * subdivided) }, 
				differences, compares
			);

			return differences;
		}));
	}

	usz finalSize = sizeof(NFSP_Header);

	for (u32 i = 0; i < thrs; ++i) {

		List<Buffer> bufs = std::move(futures[i].get());

		if (bufs.size()) {

			usz off = blocks.size();
			blocks.resize(off + bufs.size());

			for (usz j = 0; j < bufs.size(); ++j) {
				Buffer &buf = bufs[j];
				blocks[off + j] = { usz(buf.add() - modified.add()), buf.size(), buf };
				finalSize += buf.size();
			}

			finalSize += sizeof(NFSP_BlockHeader) * bufs.size();
		}
	}

	if (blocks.empty()) {
		std::printf("Couldn't patch file! As they are identical\n");
		return { };
	}

	if (blocks.size() > u32_MAX) {
		std::printf("Couldn't patch file! Too many blocks\n");
		return { };
	}

	header.blocks = u32(blocks.size());

	std::sort(blocks.begin(), blocks.end());

	//Ready to write, very simple

	Buffer result = Buffer::alloc(finalSize), resultOrigin = result;

	result.append(header);

	for (u32 i = 0; i < blocks.size(); ++i) {
		NFSP_Block &blc = blocks[i];
		result.append((NFSP_BlockHeader) blc);
		result.appendBuffer(blc.buf);
	}

	//

	t.stop();
	printf("Completed writing patch:\n");
	t.print();

	return resultOrigin;
}

bool Patcher::writePatch(const String &original, const String &modified, const String &patch) {

	Buffer og = Buffer::readFile(original);
	Buffer mod = Buffer::readFile(modified);

	if (!og.size()) {

		if (mod.size())
			mod.dealloc();

		return false;
	}

	if (!mod.size()) {

		if (og.size())
			og.dealloc();

		return false;
	}

	Buffer res = writePatch(og, mod);
	if (!res.size()) {
		mod.dealloc();
		og.dealloc();
		return false;
	}

	bool b = res.writeFile(patch);
	res.dealloc();
	og.dealloc();
	mod.dealloc();
	return b;
}