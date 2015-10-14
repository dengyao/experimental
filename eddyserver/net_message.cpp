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

void NetMessage::append(const void *user_data, size_t len)
{

}

void NetMessage::prepend(const void *user_data, size_t len)
{

}