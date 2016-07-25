#ifndef __DEFAULT_MESSAGE_FILTER_H__
#define __DEFAULT_MESSAGE_FILTER_H__

#include "MessageFilterInterface.h"

namespace network
{
	class DefaultMessageFilter : public MessageFilterInterface
	{
		DefaultMessageFilter(const DefaultMessageFilter&) = delete;
		DefaultMessageFilter& operator= (const DefaultMessageFilter&) = delete;

	public:
		typedef uint16_t MessageHeader;

	public:
		DefaultMessageFilter();

	public:
		size_t BytesWannaRead() override;

		size_t BytesWannaWrite(const std::vector<NetMessage> &messages_to_be_sent) override;

		size_t Read(const Buffer &buffer, std::vector<NetMessage> &messages_received) override;

		size_t Write(const std::vector<NetMessage> &messages_to_be_sent, Buffer &buffer) override;

	private:
		bool				header_read_;
		MessageHeader		header_;
		static const size_t	s_header_size_;
	};
}

#endif