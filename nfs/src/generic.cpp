#include "generic.hpp"
#include <fstream>

Buffer::Buffer(usz _size, u8 *_ptr) : len(_size), ptr(_ptr) {}
Buffer::Buffer() : Buffer(0, nullptr) {}

void Buffer::dealloc() {
	if (ptr) {
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

void Buffer::requireSize(usz siz) const {
	if (siz > len)
		EXCEPTION("No space left in Buffer");
}

void Buffer::appendBuffer(Buffer buf) {
	requireSize(buf.len);
	std::memcpy(ptr, buf.ptr, buf.len);
	addOffset(buf.len);
}

Buffer Buffer::appendBufferConst(Buffer buf) const {
	requireSize(buf.len);
	std::memcpy(ptr, buf.ptr, buf.len);
	return offset(buf.len);
}

void Buffer::appendString(const String &str) {
	requireSize(str.size());
	std::memcpy(ptr, str.data(), str.size());
	addOffset(str.size());
}

String Buffer::consumeString(usz l) {
	requireSize(l);
	String res = String((c8*)ptr, l);
	addOffset(l);
	return res;
}

Buffer Buffer::splitConsume(usz l) {
	requireSize(l);
	Buffer res = Buffer(l, ptr);
	addOffset(l);
	return res;
}

void Buffer::decreaseEnd(usz siz) {
	requireSize(siz);
	len -= siz;
}

Buffer Buffer::cutEnd(usz siz) const {
	requireSize(siz);
	return Buffer(len - siz, ptr);
}

Buffer Buffer::subset(usz beg, usz siz) const {
	requireSize(beg + siz);
	return Buffer(siz, ptr + beg);
}

Buffer Buffer::copyReverse() const {

	if (!len)
		return {};

	Buffer res = Buffer::alloc(len);

	for (usz i = 0, j = len - 1; i < len; ++i, --j)
		res[i] = ptr[j];

	return res;
}

Buffer Buffer::copy() const {
	return Buffer::copy(*this);
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

	if (!len)
		return false;

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