#include "NetMessage.h"
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
		EnsureWritableBytes(size);
	}

	NetMessage::NetMessage(const NetMessage &that)
		: reader_pos_(that.reader_pos_)
		, writer_pos_(that.writer_pos_)
	{
		if (!that.IsDynamic())
		{
			static_data_ = that.static_data_;
		}
		else
		{
			dynamic_data_.reset(new DynamicVector(std::move(*that.dynamic_data_)));
		}
	}

	NetMessage::NetMessage(NetMessage &&that)
		: reader_pos_(that.reader_pos_)
		, writer_pos_(that.writer_pos_)
	{
		if (!that.IsDynamic())
		{
			static_data_ = that.static_data_;
		}
		else
		{
			dynamic_data_.reset(new DynamicVector(std::move(*that.dynamic_data_)));
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
			if (!that.IsDynamic())
			{
				static_data_ = that.static_data_;
			}
			else
			{
				dynamic_data_.reset(new DynamicVector(std::move(*that.dynamic_data_)));
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
			if (!that.IsDynamic())
			{
				static_data_ = that.static_data_;
			}
			else
			{
				dynamic_data_.reset(new DynamicVector(std::move(*that.dynamic_data_)));
			}
			that.reader_pos_ = 0;
			that.writer_pos_ = 0;
		}
		return *this;
	}

	void NetMessage::Swap(NetMessage &that)
	{
		if (std::addressof(that) != this)
		{
			static_data_.swap(that.static_data_);
			dynamic_data_.swap(that.dynamic_data_);
			std::swap(reader_pos_, that.reader_pos_);
			std::swap(writer_pos_, that.writer_pos_);
		}
	}

	void NetMessage::Clear()
	{
		reader_pos_ = 0;
		writer_pos_ = 0;
		if (IsDynamic())
		{
			dynamic_data_->clear();
		}
	}

	void NetMessage::SetDynamic()
	{
		assert(!IsDynamic());
		if (IsDynamic())
		{
			return;
		}

		const size_t content_size = Readable();
		dynamic_data_.reset(new DynamicVector(content_size));
		dynamic_data_->insert(dynamic_data_->begin(), static_data_.begin() + reader_pos_, static_data_.begin() + writer_pos_);
		reader_pos_ = 0;
		writer_pos_ = content_size;
		assert(content_size == Readable());
	}

	void NetMessage::Reserve(size_t size)
	{
		if (!IsDynamic())
		{
			if (size <= static_data_.size())
			{
				return;
			}
			SetDynamic();
		}
		dynamic_data_->reserve(size);
	}

	void NetMessage::HasWritten(size_t size)
	{
		assert(Writeable() >= size);
		writer_pos_ += size;
	}

	void NetMessage::RetrieveAll()
	{
		reader_pos_ = 0;
		writer_pos_ = 0;
	}

	void NetMessage::Retrieve(size_t size)
	{
		assert(Readable() >= size);
		if (Readable() > size)
		{
			reader_pos_ += size;
		}
		else
		{
			RetrieveAll();
		}
	}

	void NetMessage::MakeSpace(size_t size)
	{
		if (Writeable() + Prependable() < size)
		{
			if (!IsDynamic())
			{
				SetDynamic();
			}
			dynamic_data_->resize(writer_pos_ + size);
		}
		else
		{
			const size_t readable_size = Readable();
			if (!IsDynamic())
			{
				memcpy(static_data_.data(), static_data_.data() + reader_pos_, readable_size);
			}
			else
			{
				memcpy(dynamic_data_->data(), dynamic_data_->data() + reader_pos_, readable_size);
			}
			reader_pos_ = 0;
			writer_pos_ = readable_size;
			assert(readable_size == Readable());
		}
	}

	void NetMessage::EnsureWritableBytes(size_t size)
	{
		if (Writeable() < size)
		{
			MakeSpace(size);
		}
		assert(Writeable() >= size);
	}

	size_t NetMessage::Write(const void *user_data, size_t size)
	{
		EnsureWritableBytes(size);
		if (!IsDynamic())
		{
			memcpy(static_data_.data() + writer_pos_, user_data, size);
		}
		else
		{
			dynamic_data_->insert(dynamic_data_->begin() + writer_pos_,
				reinterpret_cast<ConstPointer>(user_data),
				reinterpret_cast<ConstPointer>(user_data) + size);
		}
		HasWritten(size);
		return size;
	}

	int8_t NetMessage::ReadInt8()
	{
		assert(Readable() >= sizeof(int8_t));
		int8_t value = 0;
		memcpy(&value, Data(), sizeof(int8_t));
		Retrieve(sizeof(int8_t));
		return value;
	}

	int16_t NetMessage::ReadInt16()
	{
		assert(Readable() >= sizeof(int16_t));
		int16_t value = 0;
		memcpy(&value, Data(), sizeof(int16_t));
		Retrieve(sizeof(int16_t));
		return value;
	}

	int32_t NetMessage::ReadInt32()
	{
		assert(Readable() >= sizeof(int32_t));
		int32_t value = 0;
		memcpy(&value, Data(), sizeof(int32_t));
		Retrieve(sizeof(int32_t));
		return value;
	}

	int64_t NetMessage::ReadInt64()
	{
		assert(Readable() >= sizeof(int64_t));
		int64_t value = 0;
		memcpy(&value, Data(), sizeof(int64_t));
		Retrieve(sizeof(int64_t));
		return value;
	}

	uint8_t NetMessage::ReadUInt8()
	{
		assert(Readable() >= sizeof(uint8_t));
		uint8_t value = 0;
		memcpy(&value, Data(), sizeof(uint8_t));
		Retrieve(sizeof(uint8_t));
		return value;
	}

	uint16_t NetMessage::ReadUInt16()
	{
		assert(Readable() >= sizeof(uint16_t));
		uint16_t value = 0;
		memcpy(&value, Data(), sizeof(uint16_t));
		Retrieve(sizeof(uint16_t));
		return value;
	}

	uint32_t NetMessage::ReadUInt32()
	{
		assert(Readable() >= sizeof(uint32_t));
		uint32_t value = 0;
		memcpy(&value, Data(), sizeof(uint32_t));
		Retrieve(sizeof(uint32_t));
		return value;
	}

	uint64_t NetMessage::ReadUInt64()
	{
		assert(Readable() >= sizeof(uint64_t));
		uint64_t value = 0;
		memcpy(&value, Data(), sizeof(uint64_t));
		Retrieve(sizeof(uint64_t));
		return value;
	}

	std::string NetMessage::ReadString()
	{
		assert(Readable() > 0);
		const char *eos = Data();
		while (*eos++);
		size_t lenght = eos - Data() - 1;
		assert(Readable() >= lenght);
		std::string value;
		if (lenght > 0)
		{
			value.resize(lenght);
			memcpy(&*value.begin(), Data(), lenght);
			Retrieve(lenght);
		}
		return value;
	}

	std::string NetMessage::ReadLenghtAndString()
	{
		assert(Readable() >= sizeof(uint32_t));
		uint32_t lenght = 0;
		memcpy(&lenght, Data(), sizeof(uint32_t));
		Retrieve(sizeof(uint32_t));
		std::string value;
		if (lenght > 0)
		{
			value.resize(lenght);
			memcpy(&*value.begin(), Data(), lenght);
			Retrieve(lenght);
		}
		return value;
	}

	void NetMessage::WriteInt8(int8_t value)
	{
		Write(&value, sizeof(int8_t));
	}

	void NetMessage::WriteInt16(int16_t value)
	{
		Write(&value, sizeof(int16_t));
	}

	void NetMessage::WriteInt32(int32_t value)
	{
		Write(&value, sizeof(int32_t));
	}

	void NetMessage::WriteInt64(int64_t value)
	{
		Write(&value, sizeof(int64_t));
	}

	void NetMessage::WriteUInt8(uint8_t value)
	{
		Write(&value, sizeof(uint8_t));
	}

	void NetMessage::WriteUInt16(uint16_t value)
	{
		Write(&value, sizeof(uint16_t));
	}

	void NetMessage::WriteUInt32(uint32_t value)
	{
		Write(&value, sizeof(uint32_t));
	}

	void NetMessage::WriteUInt64(uint64_t value)
	{
		Write(&value, sizeof(uint64_t));
	}

	void NetMessage::WriteString(const std::string &value)
	{
		if (!value.empty())
		{
			Write(&*value.begin(), value.length());
		}
	}

	void NetMessage::WriteLenghtAndString(const std::string &value)
	{
		WriteUInt32(value.length());
		WriteString(value);
	}
}