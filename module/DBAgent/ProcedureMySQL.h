#ifndef __PROCEDURE_MYSQL_H__
#define __PROCEDURE_MYSQL_H__

#include <string>
#include <vector>

typedef std::vector<char> ByteArray;

class ProcedureMySQL
{
	ProcedureMySQL(const ProcedureMySQL&&) = delete;
	ProcedureMySQL& operator= (const ProcedureMySQL&&) = delete;

public:
	ProcedureMySQL(const ByteArray &statement);

public:
	// 是否有变量
	bool HasVariable() const;

	// 查询变量值
	ByteArray QueryVariableValue();

	// 获取变量列表
	const std::vector<std::string>& VariableList() const;

private:
	ByteArray query_variable_;
	std::vector<std::string> variable_lists_;
};

#endif
