#include <vector>
#include <cstdint>
#include <cassert>


class NetMessage final
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

public:
	explicit NetMessage(size_t initial_size = kInitialSize)
		: buffer_(kCheapPrepend + initial_size)
		, reader_pos_(kCheapPrepend)
		, writer_pos_(kCheapPrepend)
	{
		assert(readable_bytes() == 0);
		assert(writable_bytes() == initial_size);
		assert(prependable_bytes() == kCheapPrepend);
	}

public:

	size_t readable_bytes() const;

	size_t writable_bytes() const;

	size_t prependable_bytes() const;

	const uint8_t* peek() const;

	void append(const void *user_data, size_t len);

	void prepend(const void *user_data, size_t len);

private:


private:
	size_t reader_pos_;
	size_t writer_pos_;
	std::vector<uint8_t> buffer_;
};