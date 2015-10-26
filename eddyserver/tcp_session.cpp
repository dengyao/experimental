#include "tcp_session.h"

#include <cassert>
#include <iostream>
#include "id_generator.h"
#include "message_filter.h"
#include "io_service_thread.h"
#include "tcp_session_handle.h"
#include "io_service_thread_manager.h"


namespace helper
{
	typedef std::shared_ptr< std::vector<NetMessage> > NetMessageVecPointer;

	void SendMessageListToHandler(IOServiceThreadManager &manager, TCPSessionID id, NetMessageVecPointer messages)
	{
		std::shared_ptr<TCPSessionHandle> handler = manager.session_handler(id);
		if (handler == nullptr) return;

		for (auto &message : *messages)
		{
			handler->on_message(message);
		}
	}

	void PackMessageList(std::shared_ptr<TCPSession> session)
	{
		if (session->messages_received().empty()) return;

		NetMessageVecPointer messages = std::make_shared< std::vector<NetMessage> >(std::move(session->messages_received()));
		session->thread()->manager().main_thread()->post(std::bind(
			helper::SendMessageListToHandler, std::ref(session->thread()->manager()), session->id(), messages));
	}

	void SendMessageListDirectly(std::shared_ptr<TCPSession> session)
	{
		std::shared_ptr<TCPSessionHandle> handler = session->thread()->manager().session_handler(session->id());
		if (handler == nullptr) return;

		for (auto &message : session->messages_received())
		{
			handler->on_message(message);
		}
		session->messages_received().clear();
	}
}


TCPSession::TCPSession(ThreadPointer thread, MessageFilterPointer filter)
	: closed_(false)
	, thread_(thread)
	, filter_(filter)
	, num_handlers_(0)
	, socket_(thread->io_service())
	, session_id_(IDGenerator::kInvalidID)
{

}

TCPSession::~TCPSession()
{

}

void TCPSession::init(TCPSessionID id)
{
	assert(IDGenerator::kInvalidID != id);

	set_session_id(id);
	thread_->session_queue().add(shared_from_this());

	asio::ip::tcp::no_delay option(true);
	socket_.set_option(option);

	size_t bytes_wanna_read = filter_->bytes_wanna_read();
	if (bytes_wanna_read == 0) return;

	++num_handlers_;
	if (bytes_wanna_read == filter_->any_bytes())
	{
		socket_.async_read_some(asio::buffer(buffer_receiving_.peek(), buffer_receiving_.writable_bytes()),
								std::bind(&TCPSession::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	else
	{
		buffer_receiving_.make_space(bytes_wanna_read);
		socket_.async_receive(asio::buffer(buffer_receiving_.peek(), bytes_wanna_read),
							  std::bind(&TCPSession::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
}

void TCPSession::close()
{
	if (closed_)
	{
		return;
	}
	closed_ = true;
	hanlde_close();
}

void TCPSession::hanlde_close()
{
	if (num_handlers_ > 0)
	{
		return;
	}

	thread_->manager().main_thread()->post(
		std::bind(&IOServiceThreadManager::on_session_close, &thread_->manager(), id()));

	asio::error_code error_code;
	socket_.shutdown(asio::ip::tcp::socket::shutdown_both, error_code);
	if (error_code && error_code != asio::error::not_connected)
	{
		std::cerr << error_code.message() << std::endl;
	}

	socket_.close();
	thread_->session_queue().remove(id());
}

void TCPSession::post_message_list(std::vector<NetMessage> &messages)
{
	if (closed_) return;

	assert(!messages.empty());
	messages_to_be_sent_ = std::move(messages);

	size_t bytes_wanna_write = filter_->bytes_wanna_write(messages_to_be_sent_);
	if (bytes_wanna_write == 0) return;
	buffer_to_be_sent_.make_space(bytes_wanna_write);
	filter_->write(messages_to_be_sent_, buffer_to_be_sent_);

	if (buffer_sending_.readable_bytes() == 0)
	{
		++num_handlers_;
		buffer_sending_.swap(buffer_to_be_sent_);
		socket_.async_send(asio::buffer(buffer_sending_.peek(), bytes_wanna_write),
						   std::bind(&TCPSession::hanlde_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
}

void TCPSession::handle_read(asio::error_code error_code, size_t bytes_transferred)
{
	--num_handlers_;
	assert(num_handlers_ >= 0);

	if (error_code || closed_)
	{
		closed_ = true;
		hanlde_close();
		return;
	}

	bool wanna_post = messages_received_.empty();
	size_t bytes_read = filter_->read(buffer_receiving_, messages_received_);
	assert(bytes_read == bytes_transferred);
	wanna_post = wanna_post && !messages_received_.empty();

	if (wanna_post)
	{
		if (thread_->id() == thread_->manager().main_thread()->id())
		{
			helper::SendMessageListDirectly(shared_from_this());
		}
		else
		{
			thread_->post(std::bind(helper::PackMessageList, shared_from_this()));
		}
	}

	size_t bytes_wanna_read = filter_->bytes_wanna_read();
	if (bytes_wanna_read == 0) return;

	++num_handlers_;
	buffer_receiving_.retrieve_all();
	if (bytes_wanna_read == filter_->any_bytes())
	{
		socket_.async_read_some(asio::buffer(buffer_receiving_.peek(), buffer_receiving_.writable_bytes()),
								std::bind(&TCPSession::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	else
	{
		buffer_receiving_.make_space(bytes_wanna_read);
		socket_.async_receive(asio::buffer(buffer_receiving_.peek(), bytes_wanna_read),
							  std::bind(&TCPSession::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
}

void TCPSession::hanlde_write(asio::error_code error_code, size_t bytes_transferred)
{
	--num_handlers_;
	assert(num_handlers_ >= 0);

	if (error_code || closed_)
	{
		closed_ = true;
		hanlde_close();
		return;
	}

	buffer_sending_.retrieve_all();

	if (buffer_to_be_sent_.readable_bytes() == 0)
	{
		size_t bytes_wanna_write = filter_->bytes_wanna_write(messages_to_be_sent_);

		if (bytes_wanna_write == 0) return;
	
		buffer_to_be_sent_.make_space(bytes_wanna_write);
		filter_->write(messages_to_be_sent_, buffer_to_be_sent_);
	}

	++num_handlers_;
	buffer_sending_.swap(buffer_to_be_sent_);
	socket_.async_send(asio::buffer(buffer_sending_.peek(), buffer_sending_.readable_bytes()),
					   std::bind(&TCPSession::hanlde_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}