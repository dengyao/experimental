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
		bool is_dynamic() const
		{
			return dynamic_data_ != nullptr;
		}

		size_t readable() const
		{
			return writer_pos_ - reader_pos_;
		}

		size_t prependable() const
		{
			return reader_pos_;
		}

		size_t writeable() const
		{
			return is_dynamic() ? dynamic_data_->size() - writer_pos_ : static_data_.size() - writer_pos_;
		}

		size_t capacity() const
		{
			return is_dynamic() ? dynamic_data_->capacity() : static_data_.size();
		}

		bool empty() const
		{
			return readable() == 0;
		}

		Pointer data()
		{
			return (is_dynamic() ? dynamic_data_->data() : static_data_.data()) + reader_pos_;
		}

		ConstPointer data() const
		{
			return (is_dynamic() ? dynamic_data_->data() : static_data_.data()) + reader_pos_;
		}

	public:
		void clear();

		void set_dynamic();

		void reserve(size_t size);

		void swap(NetMessage &that);

		void retrieve_all();

		void retrieve(size_t size);

		size_t write(const void *user_data, size_t size);

	public:
		int8_t read_int8();

		int16_t read_int16();

		int32_t read_int32();

		int64_t read_int64();

		uint8_t read_uint8();

		uint16_t read_uint16();

		uint32_t read_uint32();

		uint64_t read_uint64();

		std::string read_string();

		std::string read_lenght_and_string();

	public:
		void write_int8(int8_t value);

		void write_int16(int16_t value);

		void write_int32(int32_t value);

		void write_int64(int64_t value);

		void write_uint8(uint8_t value);

		void write_uint16(uint16_t value);

		void write_uint32(uint32_t value);

		void write_uint64(uint64_t value);

		void write_string(const std::string &value);

		void write_lenght_and_string(const std::string &value);

	private:
		void make_space(size_t size);

		void has_written(size_t size);

		void ensure_writable_bytes(size_t size);

	private:
		size_t reader_pos_;
		size_t writer_pos_;
		std::unique_ptr<DynamicVector> dynamic_data_;
		std::array<ValueType, kDynamicThreshold> static_data_;
	};

	typedef std::vector<NetMessage> NetMessageVector;
}

#endif