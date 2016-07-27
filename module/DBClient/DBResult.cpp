#include "DBResult.h"
#include <array>
#include <limits>
#include <cassert>
#include <cstring>

namespace db
{
	static const size_t kNumRowsIndex = 0;
	static const size_t kNumFieldIndex = 1;

	/************************************************************************/

	WrapResultItem::WrapResultItem(const WrapResultSet* const result, size_t row)
		: result_(result)
		, row_(row)
	{
	}

	// 转换索引
	inline size_t WrapResultItem::ToVecIndex(size_t index) const
	{
		return 2 + (row_ + 1) * result_->NumField() + index;
	}

	inline size_t WrapResultItem::ToFieldIndex(const char *field) const
	{
		for (size_t index = 0; index < result_->NumField(); ++index)
		{
			if (strcmp(result_->rows_[2 + index], field) == 0)
			{
				return index;
			}
		}
		return result_->NumField();
	}

	// 是否为空值
	bool WrapResultItem::IsNull(size_t index) const
	{
		assert(index < result_->NumField());
		if (index >= result_->NumField())
		{
			return true;
		}
		return strlen(result_->rows_[ToVecIndex(index)]) == 0;
	}

	bool WrapResultItem::IsNull(const std::string &field) const
	{
		size_t index = ToFieldIndex(field.c_str());
		assert(index < result_->NumField());
		if (index >= result_->NumField())
		{
			return true;
		}
		return IsNull(index);
	}

	// 获取字段数据
	const char* WrapResultItem::operator[] (size_t index) const
	{
		assert(index < result_->NumField());
		if (index >= result_->NumField())
		{
			return nullptr;
		}
		return result_->rows_[ToVecIndex(index)];
	}

	const char* WrapResultItem::operator[] (const std::string &field) const
	{
		size_t index = ToFieldIndex(field.c_str());
		assert(index < result_->NumField());
		if (index >= result_->NumField())
		{
			return nullptr;
		}
		return result_->rows_[ToVecIndex(index)];
	}

	/************************************************************************/
	WrapResultSet::WrapResultSet(const char *data, size_t size)
		: num_rows_(0)
		, num_fields_(0)
	{
		Deserialize(data, size);
	}

	// 反序列化
	void WrapResultSet::Deserialize(const char *data, size_t size)
	{
		size_t index = 0;
		size_t use_size = 0;
		std::array<const char*, std::numeric_limits<uint16_t>::max()> extrabuf;
		while (use_size < size)
		{
			size_t field_size = strlen(data + use_size) + 1;
			size_t new_use_size = use_size + field_size;
			if (index < extrabuf.size())
			{
				extrabuf[index] = data + use_size;
			}
			else
			{
				if (rows_.empty())
				{
					rows_.reserve(new_use_size >= size ? index + 1 : (index + 1) * 2);
					rows_.resize(index + 1);
					memcpy(rows_.data(), extrabuf.data(), index * sizeof(decltype(extrabuf)::value_type));
				}
				else
				{
					while (index + 1 > rows_.capacity())
					{
						rows_.reserve((index + 1) * 2);
					}
					rows_.resize(index + 1);
				}
				rows_[index] = data + use_size;
			}
			use_size = new_use_size;
			++index;
		}
		assert(use_size == size);

		if (rows_.empty())
		{
			rows_.resize(index);
			memcpy(rows_.data(), extrabuf.data(), index * sizeof(decltype(rows_)::value_type));
		}

		num_rows_ = kNumRowsIndex < rows_.size() ? atoi(rows_[kNumRowsIndex]) - 1 : 0;
		num_fields_ = kNumFieldIndex < rows_.size() ? atoi(rows_[kNumFieldIndex]) : 0;
	}

	// 获取行数
	size_t WrapResultSet::NumRows() const
	{
		return num_rows_;
	}

	// 获取字段数
	size_t WrapResultSet::NumField() const
	{
		return num_fields_;
	}

	// 获取行数据
	WrapResultItem WrapResultSet::GetRow(size_t row) const
	{
		assert(row < num_rows_);
		if (row >= NumRows())
		{
			throw std::range_error("subscript out of range");
		}
		return WrapResultItem(this, row);
	}
}