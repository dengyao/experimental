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
	size_t readable_bytes() const;

	size_t writable_bytes() const;

	size_t prependable_bytes() const;

	const char* peek() const;

	void ensure_writable_bytes(size_t len);

	void append(const void *user_data, size_t len);

	void prepend(const void *user_data, size_t len);

private:
	char* begin();

	const char* begin() const;

	void make_space(size_t len);

	void has_written(size_t len);


private:
	size_t reader_pos_;
	size_t writer_pos_;
	std::vector<char> buffer_;
};