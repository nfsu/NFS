#include "generic.hpp"
#include <fstream>

Buffer::Buffer(usz _size, u8 *_ptr) : len(_size), ptr(_ptr) {}
Buffer::Buffer() : Buffer(0, nullptr) {}

void Buffer::dealloc() {
	if (ptr != nullptr) {
		std::free(ptr);
		len = 0;
		ptr = nullptr;
	}
}

Buffer Buffer::offset(usz _len) const {

	if (_len >= len) 
		return {};

	return { len - _len, ptr + _len };
}

Buffer &Buffer::addOffset(usz i) {

	if (i > len)
		EXCEPTION("addOffset out of bounds");

	if (i == len)
		return *this = {};

	return *this = offset(i);
}

u8 &Buffer::operator[](usz i) {

	if (i >= len)
		EXCEPTION("Buffer Out of bounds exception");

	return ptr[i];
}

Buffer Buffer::alloc(usz size) {

	if(void *ptr = std::malloc(size))
		return { size, (u8*) ptr };

	EXCEPTION("Out of memory");
}

Buffer Buffer::alloc(usz size, u8 *init) {
	Buffer a = alloc(size);
	std::memcpy(a.ptr, init, size);
	return a;
}

void Buffer::set(u8 val) {
	std::memset(ptr, val, len);
}

Buffer Buffer::allocEmpty(usz size) {
	Buffer b = alloc(size);
	b.set(0);
	return b;
}

Buffer Buffer::readFile(const String &str) {

	std::ifstream in(str, std::ios::binary);

	if (!in.good())
		return { 0, nullptr };

	usz length = in.rdbuf()->pubseekoff(0, std::ios_base::end);

	in.seekg(0, std::ios::beg);

	Buffer b = alloc(length);
	in.read((c8*) b.ptr, length);

	return b;
}

bool Buffer::writeFile(const String &str) {

	std::ofstream out(str, std::ios::binary);

	if (!out.good())
		return false;

	out.write((c8*) ptr, len);
	out.close();
	return true;
}

Buffer Buffer::copy(Buffer buf) {

	if (!buf.len || !buf.ptr)
		return buf;

	Buffer cpy = alloc(buf.len);
	std::memcpy(cpy.ptr, buf.ptr, buf.len);

	return cpy;
}