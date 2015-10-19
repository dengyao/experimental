#include "net_message.h"


NetMessage::NetMessage(size_t initial_size)
	: buffer_(kCheapPrepend + initial_size)
	, reader_pos_(kCheapPrepend)
	, writer_pos_(kCheapPrepend)
{
	assert(readable_bytes() == 0);
	assert(writable_bytes() == initial_size);
	assert(prependable_bytes() == kCheapPrepend);
}

NetMessage::~NetMessage()
{
}

char* NetMessage::begin()
{
	return &*buffer_.begin();
}

const char* NetMessage::begin() const
{
	return begin();
}

void NetMessage::make_space(size_t len)
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

void NetMessage::has_written(size_t len)
{
	assert(writable_bytes() >= len);
	writer_pos_ += len;
}

size_t NetMessage::readable_bytes() const
{
	return reader_pos_ - kCheapPrepend;
}

size_t NetMessage::writable_bytes() const
{
	return buffer_.size() - writer_pos_;
}

size_t NetMessage::prependable_bytes() const
{
	return reader_pos_;
}

const char* NetMessage::peek() const
{
	return begin() + reader_pos_;
}

void NetMessage::ensure_writable_bytes(size_t len)
{
	if (writable_bytes() < len)
	{
		make_space(len);
	}
	assert(writable_bytes() >= len);
}

void NetMessage::append(const void *user_data, size_t len)
{
	ensure_writable_bytes(len);
	const char *data = reinterpret_cast<const char *>(user_data);
	std::copy(data, data + len, begin() + writer_pos_);
	has_written(len);
}

void NetMessage::prepend(const void *user_data, size_t len)
{
	assert(prependable_bytes() >= len);
	reader_pos_ -= len;
	const char *data = reinterpret_cast<const char *>(user_data);
	std::copy(data, data + len, begin() + reader_pos_);
}