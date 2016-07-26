#include <memory>
#include <iostream>
#include <asio.hpp>
#include <network.h>
#include <ProtobufCodec.h>
#include <proto/client_link.pb.h>
#include <proto/client_login.pb.h>

void SendMessage(asio::ip::tcp::socket &sock, google::protobuf::Message *message)
{
	network::NetMessage buffer;
	ProtubufCodec::Encode(message, buffer);
	uint16_t length = htons((uint16_t)buffer.Readable());
	std::unique_ptr<char[]> to_be_sent(new char[sizeof(length) + buffer.Readable()]);
	memcpy(to_be_sent.get(), &length, sizeof(length));
	memcpy(to_be_sent.get() + sizeof(length), buffer.Data(), buffer.Readable());
	sock.send(asio::buffer(to_be_sent.get(), sizeof(length) + buffer.Readable()));
}

std::unique_ptr<google::protobuf::Message> ReadMessage(asio::ip::tcp::socket &sock)
{
	network::NetMessage buffer;
	size_t size =  sock.read_some(asio::buffer(buffer.Data(), buffer.kDynamicThreshold));
	buffer.HasWritten(size);
	uint16_t length = buffer.ReadUInt16();
	return ProtubufCodec::Decode(buffer);
}

int main(int argc, char *argv[])
{
	asio::io_service io_service;
	asio::ip::tcp::socket sock(io_service);
	sock.connect(asio::ip::tcp::endpoint(asio::ip::address_v4::from_string("192.168.1.109"), 10020));

	// 查询分区列表
	{
		cli::QueryPartitionReq query_part;
		SendMessage(sock, &query_part);

		auto result = ReadMessage(sock);
		assert(dynamic_cast<cli::QueryPartitionRsp*>(result.get()) != nullptr);
		cli::QueryPartitionRsp *query_part_rep = static_cast<cli::QueryPartitionRsp*>(result.get());
		for (int i = 0; i < query_part_rep->lists_size(); ++i)
		{
			auto &item = query_part_rep->lists(i);
			std::cout << item.id() << "----" << item.name() << "----" << item.status() << "----" << item.is_recommend() << std::endl;
		}
	}

	// 注册账号
	{
		/*cli::SignUpReq sign_up;
		sign_up.set_user("tes0t");
		sign_up.set_passwd("0000");
		SendMessage(sock, &sign_up);

		auto result = ReadMessage(sock);
		assert(dynamic_cast<cli::SignUpRsp*>(result.get()) != nullptr);*/
	}

	// 登录账号
	{
		cli::SignInReq sign_in;
		sign_in.set_user("tes0t");
		sign_in.set_passwd("0000");
		SendMessage(sock, &sign_in);

		auto result = ReadMessage(sock);
		assert(dynamic_cast<cli::SignInRsp*>(result.get()) != nullptr);
	}

	// 进入分区
	std::string ip;
	uint64_t token = 0;
	unsigned short port = 0;
	{
		cli::EntryPartitionReq req;
		req.set_id(1);
		SendMessage(sock, &req);

		auto result = ReadMessage(sock);
		assert(dynamic_cast<cli::EntryPartitionRsp*>(result.get()) != nullptr);
		auto part = dynamic_cast<cli::EntryPartitionRsp*>(result.get());
		ip = part->ip();
		port = part->port();
		token = part->token();
		sock.close();
	}

	// 连接分区服务器
	sock.connect(asio::ip::tcp::endpoint(asio::ip::address_v4::from_string(ip), port));


	
	// 验证身份
	{
		cli::UserAuthReq req;
		req.set_token(token);
		SendMessage(sock, &req);

		Sleep(1000);

		auto result = ReadMessage(sock);
		assert(dynamic_cast<cli::UserAuthRsp*>(result.get()) != nullptr);
	}

	system("pause");
	return 0;
}