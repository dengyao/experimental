#include <iostream>
#include "DBClient.h"


int main(int argc, char *argv[])
{
	eddy::IOServiceThreadManager threads(1);
	asio::ip::tcp::endpoint endpoint(asio::ip::address_v4::from_string("192.168.1.109"), 4235);
	
	std::unique_ptr<DBClient> db_client;
	try
	{
		db_client = std::make_unique<DBClient>(threads, endpoint, 4);
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		getchar();
	}
	db_client->AsyncSelect(DatabaseType::kMySQL, "sgs", "SELECT * FROM `actor`;", nullptr);

	threads.Run();
	
	return 0;
}