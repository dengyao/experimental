#ifndef __PROTOBUF_DISPACHER_H__
#define __PROTOBUF_DISPACHER_H__

#include <memory>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <network.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

class Callback
{
public:
	Callback() = default;

	virtual ~Callback() = default;

	virtual bool OnMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer) const = 0;

private:
	Callback(const Callback&) = delete;
	Callback& operator= (const Callback&) = delete;
};

template <typename T>
class CallbackT : public Callback
{
	static_assert(std::is_base_of<google::protobuf::Message, T>::value, "error in type");

public:
	typedef std::function<bool(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)> ProtobufMessageTCallback;

	CallbackT(const ProtobufMessageTCallback &callback)
		: callback_(callback)
	{
	}

	virtual bool OnMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer) const
	{
		assert(session != nullptr && message != nullptr);
		return callback_(session, message, buffer);
	}

private:
	ProtobufMessageTCallback callback_;
};

class ProtobufDispatcher
{
	typedef std::unordered_map<const google::protobuf::Descriptor*, std::unique_ptr<Callback> > CallbackMap;

public:
	typedef std::function<bool(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer)> ProtobufMessageCallback;

	explicit ProtobufDispatcher(const ProtobufMessageCallback &default_cb)
		: default_callback_(default_cb)
	{
	}

	bool OnProtobufMessage(network::TCPSessionHandler *session, google::protobuf::Message *message, network::NetMessage &buffer) const
	{
		CallbackMap::const_iterator found = callbacks_.find(message->GetDescriptor());
		if (found != callbacks_.end())
		{
			return found->second->OnMessage(session, message, buffer);;
		}
		return default_callback_(session, message, buffer);
	}

	template<typename T>
	void RegisterMessageCallback(const typename CallbackT<T>::ProtobufMessageTCallback &callback)
	{
		callbacks_.insert(std::make_pair(T::descriptor(), std::make_unique<CallbackT<T> >(callback)));
	}

private:
	CallbackMap				callbacks_;
	ProtobufMessageCallback	default_callback_;
};

#endif
