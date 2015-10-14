﻿#include <vector>
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

	void ensure_writable_bytes(size_t len)
	{
		if (writable_bytes() < len)
		{
			make_space(len);
		}
		assert(writable_bytes() >= len);
	}

	void append(const void *user_data, size_t len);

	void prepend(const void *user_data, size_t len);

private:
	void make_space(size_t len)
	{
		if (writable_bytes() + prependable_bytes() < len + kCheapPrepend)
		{
			buffer_.resize(writer_pos_ + len);
		}
		else
		{
			assert(kCheapPrepend < reader_pos_);
			size_t readable = readable_bytes();
			std::copy(begin() + reader_pos_, begin() + writer_pos_, begin() + kCheapPrepend);
			reader_pos_ = kCheapPrepend;
			writer_pos_ = reader_pos_ + readable;
			assert(readable == readable_bytes());
		}
	}

	void has_written(size_t len)
	{
		assert(writable_bytes() >= len);
		writer_pos_ += len;
	}

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