#pragma once

#include <vector>
#include <string>
#include <cstdint>


namespace eddy
{
	class NetMessage
	{
	public:
		static const size_t kCheapPrepend = 8;
		static const size_t kInitialSize = 1024;

	public:
		explicit NetMessage(size_t initial_size = kInitialSize);

		~NetMessage();

		NetMessage(const NetMessage &that);

		NetMessage(NetMessage &&that);

		NetMessage& operator= (const NetMessage &that);

		NetMessage& operator= (NetMessage &&that);

		void swap(NetMessage &that);

	public:
		size_t readable_bytes() const;

		size_t writable_bytes() const;

		size_t prependable_bytes() const;

		void make_space(size_t len);

		char* peek();

		const char* peek() const;

		void retrieve(size_t len);

		void retrieve_all();

		void append(const void *user_data, size_t len);

		void prepend(const void *user_data, size_t len);

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
		char* begin();

		const char* begin() const;

		void has_written(size_t len);

		void ensure_writable_bytes(size_t len);

	private:
		size_t reader_pos_;
		size_t writer_pos_;
		std::vector<char> buffer_;
	};
}