#ifndef __CRC32_H__
#define __CRC32_H__

#include <vector>
#include <string>
#include <cstdint>

class CRC32 final
{
	CRC32(const CRC32&) = delete;
	CRC32& operator= (const CRC32&) = delete;

public:
	CRC32();

	CRC32(const std::string &item);

	CRC32(const std::vector<char> &item);

	CRC32(const unsigned char *input, size_t length);

	~CRC32() = default;

public:
	void Clear();

	void Append(const std::string &item);

	void Append(const std::vector<char> &item);

	void Append(const unsigned char *input, size_t length);

	operator uint32_t() const;

	uint32_t CheckSum() const;

private:
	uint32_t Table(size_t idx) const;

private:
	uint32_t checksum_;
};

#endif
