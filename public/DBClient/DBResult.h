#ifndef __DBRESULT_H__
#define __DBRESULT_H__

#include <vector>
#include <string>

namespace db
{
	class WrapResultSet;

	class WrapResultItem
	{
	public:
		WrapResultItem(const WrapResultSet* const, size_t row);

	public:
		// 是否为空值
		bool IsNull(size_t index) const;
		bool IsNull(const std::string &field) const;

		// 获取字段数据
		const char* operator[] (size_t index) const;
		const char* operator[] (const std::string &field) const;

	private:
		// 转换索引
		size_t ToVecIndex(size_t index) const;
		size_t ToFieldIndex(const char *field) const;

	private:
		const size_t         row_;
		const WrapResultSet* const result_;
	};

	class WrapResultSet
	{
		friend class WrapResultItem;

	public:
		WrapResultSet(const char *data, size_t size);

	public:
		// 获取行数
		size_t NumRows() const;

		// 获取字段数
		size_t NumField() const;

		// 获取行数据
		WrapResultItem GetRow(size_t row = 0) const;

	private:
		// 反序列化
		void Deserialize(const char *data, size_t size);

	private:
		WrapResultSet(const WrapResultSet&) = delete;
		WrapResultSet operator= (const WrapResultSet&) = delete;

	private:
		size_t                   num_rows_;
		size_t                   num_fields_;
		std::vector<const char*> rows_;
	};
}

#endif