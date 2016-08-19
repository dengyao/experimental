#include "ProcedureMySQL.h"
#include <numeric>
#include <cstring>

namespace mysql_stuff
{
	struct Slice
	{
		int start;
		int end;

		Slice(int start_val, int end_val)
			: start(start_val), end(end_val)
		{
		}
	};

	// 是否是空白字符
	inline bool IsBlankCharacter(const char ch)
	{
		return (ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t');
	}

	// 获取语句中的变量
	static void GetVariableLists(const std::string &call_procedure, std::vector<Slice> *out_lists)
	{
		for (size_t i = 1; i < call_procedure.size(); ++i)
		{
			if (call_procedure[i] == '@' && (call_procedure[i - 1] == ',' || IsBlankCharacter(call_procedure[i - 1])))
			{
				for (size_t j = i + 1; j < call_procedure.size(); ++j)
				{
					if (call_procedure[j] == ' ' || call_procedure[j] == ',' || call_procedure[j] == ')')
					{
						out_lists->push_back(Slice(i, j));
						i = j + 1;
						break;
					}
				}
			}
		}
	}

	// 获取语句中的引号区间
	static void GetQuotesAround(const std::string &call_procedure, const char quotes, std::vector<Slice> *out_lists)
	{
		for (size_t i = 0; i < call_procedure.size(); ++i)
		{
			if (call_procedure[i] == quotes)
			{
				for (size_t j = i + 1; j < call_procedure.size(); ++j)
				{
					if (call_procedure[j] == quotes)
					{
						out_lists->push_back(Slice(i, j));
						i = j + 1;
						break;
					}
				}
			}
		}
	}

	// 获取引号之外的变量
	static std::vector<std::string> GetDisjointVariableLists(const std::string &call_procedure, const std::vector<Slice> &variable_lists, const std::vector<Slice> &segment_lists)
	{
		std::vector<std::string> variables;
		std::vector<Slice> clone_lists = variable_lists;
		for (size_t i = 0; i < segment_lists.size(); ++i)
		{
			for (auto iter = clone_lists.begin(); iter != clone_lists.end();)
			{
				if (segment_lists[i].start >= iter->start &&
					segment_lists[i].start <= iter->end)
				{
					iter = clone_lists.erase(iter);
				}
				else if (segment_lists[i].start <= iter->start &&
					segment_lists[i].end >= iter->start)
				{
					iter = clone_lists.erase(iter);
				}
				else
				{
					++iter;
				}
			}
		}

		for (const auto &item : clone_lists)
		{
			variables.push_back(std::string(call_procedure.data() + item.start, item.end - item.start));
		}
		return variables;
	}

	// 生成查询语句
	static ByteArray GenerateQueryStatement(const std::vector<std::string> &variables)
	{
		ByteArray statement;
		if (!variables.empty())
		{
			const char semicolon = ';';
			const char *separator = ", ";
			const char *action = "SELECT ";
			const int length = strlen(action);
			const int sep_length = strlen(separator);
			statement.resize(sizeof(semicolon) + length + (variables.size() - 1) * sep_length +
				std::accumulate(variables.begin(), variables.end(), 0, [=](size_t sum, const std::string &variable)
			{
				return sum + variable.size();
			}));

			memcpy(const_cast<char*>(statement.data()), action, length);
			size_t use_size = length;
			for (size_t i = 0; i < variables.size(); ++i)
			{
				memcpy(const_cast<char*>(statement.data()) + use_size, variables[i].data(), variables[i].size());
				use_size += variables[i].size();
				if (i != variables.size() - 1)
				{
					memcpy(const_cast<char*>(statement.data()) + use_size, separator, sep_length);
					use_size += sep_length;
				}
				else
				{
					statement[use_size] = semicolon;
				}
			}
		}
		return statement;
	}
}

/************************************************************************/

ProcedureMySQL::ProcedureMySQL(const std::string &statement)
{
	std::vector<mysql_stuff::Slice> segment_lists;
	std::vector<mysql_stuff::Slice> variable_lists;
	GetVariableLists(statement, &variable_lists);
	GetQuotesAround(statement, '\'', &segment_lists);
	GetQuotesAround(statement, '\"', &segment_lists);
	variable_lists_ = GetDisjointVariableLists(statement, variable_lists, segment_lists);	
}

// 是否有变量
bool ProcedureMySQL::HasVariable() const
{
	return !variable_lists_.empty();
}

// 查询变量值
ByteArray ProcedureMySQL::QueryVariableValue()
{
	if (variable_lists_.empty())
	{
		return ByteArray();
	}
	return mysql_stuff::GenerateQueryStatement(variable_lists_);
}

// 获取变量列表
const std::vector<std::string>& ProcedureMySQL::VariableList() const
{
	return variable_lists_;
}