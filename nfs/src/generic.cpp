#include "generic.h"
#include <fstream>

Buffer::Buffer(u32 _size, u8 *_ptr) : size(_size), ptr(_ptr) {}
Buffer::Buffer() : Buffer(0, nullptr) {}

u8 *Buffer::end() { return size + ptr; }

void Buffer::dealloc() {
	if (ptr != nullptr) {
		size = 0;
		ptr = nullptr;
		free(ptr);
	}
}

Buffer Buffer::offset(u32 len) {
	if (len >= size) return {};
	return { size - len, ptr + len };
}

Buffer &Buffer::addOffset(u32 i) {
	if (i >= size) return *this;
	return *this = offset(i);
}

u8 &Buffer::operator[](u32 i) {

	if (i >= size)
		throw std::exception("Buffer Out of bounds exception");

	return ptr[i];
}

Buffer Buffer::alloc(u32 size) {
	return { size, (u8*)malloc(size) };
}

Buffer Buffer::read(std::string str) {

	std::ifstream in(str, std::ios::binary);

	if (!in.good())
		return { 0, nullptr };

	u32 length = (u32)in.rdbuf()->pubseekoff(0, std::ios_base::end);

	in.seekg(0, std::ios::beg);

	Buffer b = alloc(length);
	in.read((char*)b.ptr, length);

	return b;
}

bool Buffer::write(std::string str) {

	std::ofstream out(str, std::ios::binary);

	if (!out.good())
		return false;

	out.write((char*)ptr, size);
	out.close();
	return true;
}

Buffer Buffer::copy(Buffer buf) {
	if (buf.size == 0 || buf.ptr == nullptr)
		return buf;

	Buffer cpy = alloc(buf.size);
	memcpy(cpy.ptr, buf.ptr, buf.size);
	return cpy;
}