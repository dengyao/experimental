#include "net_message.h"

#include <cassert>
#include <cstring>


namespace eddy
{
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

	NetMessage::NetMessage(const NetMessage &that)
		: buffer_(that.buffer_)
		, reader_pos_(that.reader_pos_)
		, writer_pos_(that.writer_pos_)
	{
	}

	NetMessage::NetMessage(NetMessage &&that)
		: buffer_(std::move(that.buffer_))
		, reader_pos_(that.reader_pos_)
		, writer_pos_(that.writer_pos_)
	{
		that.buffer_.resize(kInitialSize);
		that.reader_pos_ = kCheapPrepend;
		that.writer_pos_ = kCheapPrepend;
	}

	NetMessage& NetMessage::operator= (const NetMessage &that)
	{
		buffer_ = that.buffer_;
		reader_pos_ = that.reader_pos_;
		writer_pos_ = that.writer_pos_;
		return *this;
	}

	NetMessage& NetMessage::operator= (NetMessage &&that)
	{
		std::vector<char> &&origin_buffer = std::move(buffer_);

		buffer_ = std::move(that.buffer_);
		reader_pos_ = that.reader_pos_;
		writer_pos_ = that.writer_pos_;

		that.buffer_ = origin_buffer;
		that.reader_pos_ = kCheapPrepend;
		that.writer_pos_ = kCheapPrepend;
		return *this;
	}

	void NetMessage::swap(NetMessage &that)
	{
		size_t origin_read_pos = reader_pos_;
		size_t origin_write_pos = writer_pos_;
		std::vector<char> &&origin_buffer = std::move(buffer_);

		buffer_ = std::move(that.buffer_);
		reader_pos_ = that.reader_pos_;
		writer_pos_ = that.writer_pos_;

		that.buffer_ = origin_buffer;
		that.reader_pos_ = origin_read_pos;
		that.writer_pos_ = origin_write_pos;
	}

	char* NetMessage::begin()
	{
		return &*buffer_.begin();
	}

	const char* NetMessage::begin() const
	{
		return &*buffer_.begin();
	}

	void NetMessage::make_space(size_t len)
	{
		if (writable_bytes() + prependable_bytes() < len + kCheapPrepend)
		{
			buffer_.resize(writer_pos_ + len);
		}
		else
		{
			assert(kCheapPrepend <= reader_pos_);
			size_t readable = readable_bytes();
			::memcpy(begin() + kCheapPrepend, begin() + reader_pos_, readable);
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
		return writer_pos_ - reader_pos_;
	}

	size_t NetMessage::writable_bytes() const
	{
		return buffer_.size() - writer_pos_;
	}

	size_t NetMessage::prependable_bytes() const
	{
		return reader_pos_;
	}

	char* NetMessage::peek()
	{
		return begin() + reader_pos_;
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
		::memcpy(begin() + writer_pos_, data, len);
		has_written(len);
	}

	void NetMessage::prepend(const void *user_data, size_t len)
	{
		assert(prependable_bytes() >= len);
		reader_pos_ -= len;
		const char *data = reinterpret_cast<const char *>(user_data);
		::memcpy(begin() + reader_pos_, data, len);
	}

	void NetMessage::retrieve_all()
	{
		reader_pos_ = kCheapPrepend;
		writer_pos_ = kCheapPrepend;
	}

	void NetMessage::retrieve(size_t len)
	{
		assert(readable_bytes() >= len);
		if (readable_bytes() > len)
		{
			reader_pos_ += len;
		}
		else
		{
			retrieve_all();
		}
	}

	int8_t NetMessage::read_int8()
	{
		assert(readable_bytes() >= sizeof(int8_t));
		int8_t value = 0;
		::memcpy(&value, peek(), sizeof(int8_t));
		retrieve(sizeof(int8_t));
		return value;
	}

	int16_t NetMessage::read_int16()
	{
		assert(readable_bytes() >= sizeof(int16_t));
		int16_t value = 0;
		::memcpy(&value, peek(), sizeof(int16_t));
		retrieve(sizeof(int16_t));
		return value;
	}

	int32_t NetMessage::read_int32()
	{
		assert(readable_bytes() >= sizeof(int32_t));
		int32_t value = 0;
		::memcpy(&value, peek(), sizeof(int32_t));
		retrieve(sizeof(int32_t));
		return value;
	}

	int64_t NetMessage::read_int64()
	{
		assert(readable_bytes() >= sizeof(int64_t));
		int64_t value = 0;
		::memcpy(&value, peek(), sizeof(int64_t));
		retrieve(sizeof(int64_t));
		return value;
	}

	uint8_t NetMessage::read_uint8()
	{
		assert(readable_bytes() >= sizeof(uint8_t));
		uint8_t value = 0;
		::memcpy(&value, peek(), sizeof(uint8_t));
		retrieve(sizeof(uint8_t));
		return value;
	}

	uint16_t NetMessage::read_uint16()
	{
		assert(readable_bytes() >= sizeof(uint16_t));
		uint16_t value = 0;
		::memcpy(&value, peek(), sizeof(uint16_t));
		retrieve(sizeof(uint16_t));
		return value;
	}

	uint32_t NetMessage::read_uint32()
	{
		assert(readable_bytes() >= sizeof(uint32_t));
		uint32_t value = 0;
		::memcpy(&value, peek(), sizeof(uint32_t));
		retrieve(sizeof(uint32_t));
		return value;
	}

	uint64_t NetMessage::read_uint64()
	{
		assert(readable_bytes() >= sizeof(uint64_t));
		uint64_t value = 0;
		::memcpy(&value, peek(), sizeof(uint64_t));
		retrieve(sizeof(uint64_t));
		return value;
	}

	std::string NetMessage::read_string()
	{
		assert(readable_bytes() > 0);
		const char *eos = peek();
		while (*eos++);
		size_t lenght = eos - peek() - 1;
		assert(readable_bytes() >= lenght);
		std::string value;
		if (lenght > 0)
		{
			value.resize(lenght);
			::memcpy(&*value.begin(), peek(), lenght);
			retrieve(lenght);
		}
		return value;
	}

	std::string NetMessage::read_lenght_and_string()
	{
		assert(readable_bytes() >= sizeof(uint32_t));
		uint32_t lenght = 0;
		::memcpy(&lenght, peek(), sizeof(uint32_t));
		retrieve(sizeof(uint32_t));
		std::string value;
		if (lenght > 0)
		{
			value.resize(lenght);
			::memcpy(&*value.begin(), peek(), lenght);
			retrieve(lenght);
		}
		return value;
	}

	void NetMessage::write_int8(int8_t value)
	{
		append(&value, sizeof(int8_t));
	}

	void NetMessage::write_int16(int16_t value)
	{
		append(&value, sizeof(int16_t));
	}

	void NetMessage::write_int32(int32_t value)
	{
		append(&value, sizeof(int32_t));
	}

	void NetMessage::write_int64(int64_t value)
	{
		append(&value, sizeof(int64_t));
	}

	void NetMessage::write_uint8(uint8_t value)
	{
		append(&value, sizeof(uint8_t));
	}

	void NetMessage::write_uint16(uint16_t value)
	{
		append(&value, sizeof(uint16_t));
	}

	void NetMessage::write_uint32(uint32_t value)
	{
		append(&value, sizeof(uint32_t));
	}

	void NetMessage::write_uint64(uint64_t value)
	{
		append(&value, sizeof(uint64_t));
	}

	void NetMessage::write_string(const std::string &value)
	{
		if (!value.empty())
		{
			append(&*value.begin(), value.length());
		}
	}

	void NetMessage::write_lenght_and_string(const std::string &value)
	{
		write_uint32(value.length());
		write_string(value);
	}
}