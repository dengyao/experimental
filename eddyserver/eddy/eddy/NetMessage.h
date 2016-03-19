#ifndef __NET_MESSAGE_H__
#define __NET_MESSAGE_H__

#include <array>
#include <memory>
#include <vector>
#include <string>
#include <cstdint>

namespace eddy
{
	class NetMessage
	{
	public:
		typedef char					ValueType;
		typedef ValueType*				Pointer;
		typedef const ValueType*		ConstPointer;
		typedef std::vector<ValueType>	DynamicVector;

	public:
		static const size_t kDynamicThreshold = 128;

	public:
		NetMessage();

		NetMessage(size_t size);

		NetMessage(const NetMessage &that);

		NetMessage(NetMessage &&that);

		~NetMessage();

		NetMessage& operator= (const NetMessage &that);

		NetMessage& operator= (NetMessage &&that);

	public:
		bool IsDynamic() const
		{
			return dynamic_data_ != nullptr;
		}

		size_t Readable() const
		{
			return writer_pos_ - reader_pos_;
		}

		size_t Prependable() const
		{
			return reader_pos_;
		}

		size_t Writeable() const
		{
			return IsDynamic() ? dynamic_data_->size() - writer_pos_ : static_data_.size() - writer_pos_;
		}

		size_t Capacity() const
		{
			return IsDynamic() ? dynamic_data_->capacity() : static_data_.size();
		}

		bool Empty() const
		{
			return Readable() == 0;
		}

		Pointer Data()
		{
			return (IsDynamic() ? dynamic_data_->data() : static_data_.data()) + reader_pos_;
		}

		ConstPointer Data() const
		{
			return (IsDynamic() ? dynamic_data_->data() : static_data_.data()) + reader_pos_;
		}

	public:
		void Clear();

		void SetDynamic();

		void Reserve(size_t size);

		void Swap(NetMessage &that);

		void RetrieveAll();

		void Retrieve(size_t size);

		size_t Write(const void *user_data, size_t size);

	public:
		int8_t ReadInt8();

		int16_t ReadInt16();

		int32_t ReadInt32();

		int64_t ReadInt64();

		uint8_t ReadUInt8();

		uint16_t ReadUInt16();

		uint32_t ReadUInt32();

		uint64_t ReadUInt64();

		std::string ReadString();

		std::string ReadLenghtAndString();

	public:
		void WriteInt8(int8_t value);

		void WriteInt16(int16_t value);

		void WriteInt32(int32_t value);

		void WriteInt64(int64_t value);

		void WriteUInt8(uint8_t value);

		void WriteUInt16(uint16_t value);

		void WriteUInt32(uint32_t value);

		void WriteUInt64(uint64_t value);

		void WriteString(const std::string &value);

		void WriteLenghtAndString(const std::string &value);

	private:
		void MakeSpace(size_t size);

		void HasWritten(size_t size);

		void EnsureWritableBytes(size_t size);

	private:
		size_t										reader_pos_;
		size_t										writer_pos_;
		std::unique_ptr<DynamicVector>				dynamic_data_;
		std::array<ValueType, kDynamicThreshold>	static_data_;
	};

	typedef std::vector<NetMessage> NetMessageVector;
}

#endif