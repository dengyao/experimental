#include "net_message.h"
#include <cassert>
#include <cstring>

namespace eddy
{
	NetMessage::NetMessage()
		: reader_pos_(0)
		, writer_pos_(0)
	{
	}

	NetMessage::NetMessage(size_t size)
		: reader_pos_(0)
		, writer_pos_(0)
	{
		ensure_writable_bytes(size);
	}

	NetMessage::~NetMessage()
	{
	}

	NetMessage::NetMessage(const NetMessage &that)
		: reader_pos_(that.reader_pos_)
		, writer_pos_(that.writer_pos_)
	{
		if (!that.is_dynamic())
		{
			static_data_ = that.static_data_;
		}
		else
		{
			dynamic_data_ = std::make_unique<DynamicVector>(*that.dynamic_data_);
		}
	}

	NetMessage::NetMessage(NetMessage &&that)
		: reader_pos_(that.reader_pos_)
		, writer_pos_(that.writer_pos_)
	{
		if (!that.is_dynamic())
		{
			static_data_ = that.static_data_;
		}
		else
		{
			dynamic_data_ = std::make_unique<DynamicVector>(std::move(*that.dynamic_data_));
		}
		that.reader_pos_ = 0;
		that.writer_pos_ = 0;
	}

	NetMessage& NetMessage::operator= (const NetMessage &that)
	{
		if (std::addressof(that) != this)
		{
			reader_pos_ = that.reader_pos_;
			writer_pos_ = that.writer_pos_;
			if (!that.is_dynamic())
			{
				static_data_ = that.static_data_;
			}
			else
			{
				dynamic_data_ = std::make_unique<DynamicVector>(*that.dynamic_data_);
			}
		}
		return *this;
	}

	NetMessage& NetMessage::operator= (NetMessage &&that)
	{
		if (std::addressof(that) != this)
		{
			reader_pos_ = that.reader_pos_;
			writer_pos_ = that.writer_pos_;
			if (!that.is_dynamic())
			{
				static_data_ = that.static_data_;
			}
			else
			{
				dynamic_data_ = std::make_unique<DynamicVector>(std::move(*that.dynamic_data_));
			}
			that.reader_pos_ = 0;
			that.writer_pos_ = 0;
		}
		return *this;
	}

	void NetMessage::swap(NetMessage &that)
	{
		if (std::addressof(that) != this)
		{
			static_data_.swap(that.static_data_);
			dynamic_data_.swap(that.dynamic_data_);
			std::swap(reader_pos_, that.reader_pos_);
			std::swap(writer_pos_, that.writer_pos_);
		}
	}

	void NetMessage::clear()
	{
		reader_pos_ = 0;
		writer_pos_ = 0;
		if (is_dynamic())
		{
			dynamic_data_->clear();
		}
	}

	void NetMessage::set_dynamic()
	{
		assert(!is_dynamic());
		if (is_dynamic()) return;
		const size_t content_size = readable();
		dynamic_data_ = std::make_unique<DynamicVector>(content_size);
		dynamic_data_->insert(dynamic_data_->begin(), static_data_.begin() + reader_pos_, static_data_.begin() + writer_pos_);
		reader_pos_ = 0;
		writer_pos_ = content_size;
		assert(content_size == readable());
	}

	void NetMessage::reserve(size_t size)
	{
		if (!is_dynamic())
		{
			if (size <= static_data_.size())
			{
				return;
			}
			set_dynamic();
		}
		dynamic_data_->reserve(size);
	}

	void NetMessage::has_written(size_t size)
	{
		assert(writeable() >= size);
		writer_pos_ += size;
	}

	void NetMessage::retrieve_all()
	{
		reader_pos_ = 0;
		writer_pos_ = 0;
	}

	void NetMessage::retrieve(size_t size)
	{
		assert(readable() >= size);
		if (readable() > size)
		{
			reader_pos_ += size;
		}
		else
		{
			retrieve_all();
		}
	}

	void NetMessage::make_space(size_t size)
	{
		if (writeable() + prependable() < size)
		{
			if (!is_dynamic())
			{
				set_dynamic();
			}
			dynamic_data_->resize(writer_pos_ + size);
		}
		else
		{
			const size_t readable_size = readable();
			if (!is_dynamic())
			{
				memcpy(static_data_.data(), static_data_.data() + reader_pos_, readable_size);
			}
			else
			{
				memcpy(dynamic_data_->data(), dynamic_data_->data() + reader_pos_, readable_size);
			}
			reader_pos_ = 0;
			writer_pos_ = readable_size;
			assert(readable_size == readable());
		}
	}

	void NetMessage::ensure_writable_bytes(size_t size)
	{
		if (writeable() < size)
		{
			make_space(size);
		}
		assert(writeable() >= size);
	}

	size_t NetMessage::write(const void *user_data, size_t size)
	{
		ensure_writable_bytes(size);
		if (!is_dynamic())
		{
			memcpy(static_data_.data() + writer_pos_, user_data, size);
		}
		else
		{
			dynamic_data_->insert(dynamic_data_->begin() + writer_pos_,
								  reinterpret_cast<ConstPointer>(user_data),
								  reinterpret_cast<ConstPointer>(user_data)+size);
		}
		has_written(size);
		return size;
	}

	int8_t NetMessage::read_int8()
	{
		assert(readable() >= sizeof(int8_t));
		int8_t value = 0;
		memcpy(&value, data(), sizeof(int8_t));
		retrieve(sizeof(int8_t));
		return value;
	}

	int16_t NetMessage::read_int16()
	{
		assert(readable() >= sizeof(int16_t));
		int16_t value = 0;
		memcpy(&value, data(), sizeof(int16_t));
		retrieve(sizeof(int16_t));
		return value;
	}

	int32_t NetMessage::read_int32()
	{
		assert(readable() >= sizeof(int32_t));
		int32_t value = 0;
		memcpy(&value, data(), sizeof(int32_t));
		retrieve(sizeof(int32_t));
		return value;
	}

	int64_t NetMessage::read_int64()
	{
		assert(readable() >= sizeof(int64_t));
		int64_t value = 0;
		memcpy(&value, data(), sizeof(int64_t));
		retrieve(sizeof(int64_t));
		return value;
	}

	uint8_t NetMessage::read_uint8()
	{
		assert(readable() >= sizeof(uint8_t));
		uint8_t value = 0;
		memcpy(&value, data(), sizeof(uint8_t));
		retrieve(sizeof(uint8_t));
		return value;
	}

	uint16_t NetMessage::read_uint16()
	{
		assert(readable() >= sizeof(uint16_t));
		uint16_t value = 0;
		memcpy(&value, data(), sizeof(uint16_t));
		retrieve(sizeof(uint16_t));
		return value;
	}

	uint32_t NetMessage::read_uint32()
	{
		assert(readable() >= sizeof(uint32_t));
		uint32_t value = 0;
		memcpy(&value, data(), sizeof(uint32_t));
		retrieve(sizeof(uint32_t));
		return value;
	}

	uint64_t NetMessage::read_uint64()
	{
		assert(readable() >= sizeof(uint64_t));
		uint64_t value = 0;
		memcpy(&value, data(), sizeof(uint64_t));
		retrieve(sizeof(uint64_t));
		return value;
	}

	std::string NetMessage::read_string()
	{
		assert(readable() > 0);
		const char *eos = data();
		while (*eos++);
		size_t lenght = eos - data() - 1;
		assert(readable() >= lenght);
		std::string value;
		if (lenght > 0)
		{
			value.resize(lenght);
			memcpy(&*value.begin(), data(), lenght);
			retrieve(lenght);
		}
		return value;
	}

	std::string NetMessage::read_lenght_and_string()
	{
		assert(readable() >= sizeof(uint32_t));
		uint32_t lenght = 0;
		memcpy(&lenght, data(), sizeof(uint32_t));
		retrieve(sizeof(uint32_t));
		std::string value;
		if (lenght > 0)
		{
			value.resize(lenght);
			memcpy(&*value.begin(), data(), lenght);
			retrieve(lenght);
		}
		return value;
	}

	void NetMessage::write_int8(int8_t value)
	{
		write(&value, sizeof(int8_t));
	}

	void NetMessage::write_int16(int16_t value)
	{
		write(&value, sizeof(int16_t));
	}

	void NetMessage::write_int32(int32_t value)
	{
		write(&value, sizeof(int32_t));
	}

	void NetMessage::write_int64(int64_t value)
	{
		write(&value, sizeof(int64_t));
	}

	void NetMessage::write_uint8(uint8_t value)
	{
		write(&value, sizeof(uint8_t));
	}

	void NetMessage::write_uint16(uint16_t value)
	{
		write(&value, sizeof(uint16_t));
	}

	void NetMessage::write_uint32(uint32_t value)
	{
		write(&value, sizeof(uint32_t));
	}

	void NetMessage::write_uint64(uint64_t value)
	{
		write(&value, sizeof(uint64_t));
	}

	void NetMessage::write_string(const std::string &value)
	{
		if (!value.empty())
		{
			write(&*value.begin(), value.length());
		}
	}

	void NetMessage::write_lenght_and_string(const std::string &value)
	{
		write_uint32(value.length());
		write_string(value);
	}
}