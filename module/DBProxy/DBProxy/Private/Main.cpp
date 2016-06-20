#include <iostream>
#include <rapidjson/document.h>
#include <DBProxy/Private/ConnectorMySQL.h>
#include <DBProxy/Private/ProxyManager.h>
#include <functional>


 dbproxy::ConnectorMySQLPointer CreateConnectorMySQLHandle()
{
	dbproxy::ConnectorMySQLPointer connector;
	try
	{
		connector = std::make_shared<dbproxy::ConnectorMySQL>("192.168.1.201", 3306, "root", "123456", 5);
	}
	catch (std::exception* e)
	{
		
	}
}

int main(int argc, char *argv[])
{
	dbproxy::ProxyManager<dbproxy::MySQL> proxy(std::bind(creator), 10);
	 connector;
	try
	{
		dbproxy::ErrorCode error;
		

		connector->SetCharacterSet("utf8", error);
		if (error)
		{
			std::cout << "ÉèÖÃ×Ö·û±àÂëÊ§°Ü£¡" << std::endl;
		}

		connector->SelectDatabase("sgs", error);
		if (error)
		{
			std::cout << "Ñ¡ÔñÊý¾Ý¿âÊ§°Ü£¡" << std::endl;
		}

		auto result = connector->Select("SELECT * FROM `actor`;", error);
		if (error)
		{
			std::cout << "²éÑ¯Êý¾Ý¿âÊ§°Ü£¡" << std::endl;
		}
		else
		{
			dbproxy::ResultWrap wrap = result.Wrap();
			for (size_t row = 0; row < wrap.NumRows(); ++row)
			{
				for (size_t col = 0; col < wrap.NumField(); ++col)
				{
					std::cout << wrap.Value(row, col) << std::endl;
				}
				std::cout << "---------------------------------------" << std::endl;
			}
		}

	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}

	system("pause");
	return 0;
}