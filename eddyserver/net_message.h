#include <vector>
#include <cstdint>
#include <cassert>


class NetMessage final
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

public:
	explicit NetMessage(size_t initial_size = kInitialSize);
	~NetMessage();

public:
	size_t readable_bytes() const
	{
		return writer_pos_ - reader_pos_;
	}

	size_t writable_bytes() const
	{
		return buffer_.size() - writer_pos_;
	}

	size_t prependable_bytes() const
	{
		return reader_pos_;
	}

	const uint8_t* peek() const
	{
		return begin() + reader_pos_;
	}

	void ensure_writable_bytes(size_t len);

	void append(const void *user_data, size_t len);

	void prepend(const void *user_data, size_t len);

private:
	void make_space(size_t len);

	void has_written(size_t len);

	uint8_t* begin()
	{
		return &*buffer_.begin();
	}

	const uint8_t* begin() const
	{
		return &*buffer_.begin();
	}

private:
	size_t reader_pos_;
	size_t writer_pos_;
	std::vector<uint8_t> buffer_;
};