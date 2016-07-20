#ifndef __GLOBAL_OBJECT_H__
#define __GLOBAL_OBJECT_H__

#include <memory>

namespace db
{
	class DBClient;
}

const std::unique_ptr<db::DBClient>& GlobalDBClient();

void OnceInitGlobalDBClient(std::unique_ptr<db::DBClient> &&db_client);


#endif
