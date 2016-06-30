#ifndef __WRAP_RESULT_H__
#define __WRAP_RESULT_H__

#include <array>
#include <vector>
#include <cstring>
#include <cassert>

namespace dbproxy
{
	class WrapResult
	{
		static const size_t kNumRowsIndex = 0;
		static const size_t kNumFieldIndex = 1;

	public:
		WrapResult(const char *data, size_t size)
			: num_rows_(0)
			, num_fields_(0)
		{
			Deserialize(data, size);
		}

		WrapResult(WrapResult &&other)
			: num_rows_(other.num_rows_)
			, num_fields_(other.num_fields_)
		{
			other.num_rows_ = 0;
			other.num_fields_ = 0;
			rows_ = std::move(other.rows_);
		}

	public:
		size_t NumRows() const
		{
			return num_rows_;
		}

		size_t NumField() const
		{
			return num_fields_;
		}

		const char* Value(size_t row, size_t col) const
		{
			assert(row < num_rows_ && col < num_fields_);
			if (row >= NumRows() || col >= NumField())
			{
				throw std::range_error("subscript out of range");
			}
			size_t index = row * NumField() + col;
			return rows_[index + 2];
		}

		size_t Length(size_t row, size_t col) const
		{
			return strlen(Value(row, col));
		}

	private:
		void Deserialize(const char *data, size_t size)
		{
			size_t index = 0;
			size_t use_size = 0;
			std::array<const char*, 65535> extrabuf;
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
						rows_[index] = data + use_size;
					}
					else
					{
						while (index + 1 > rows_.capacity())
						{
							rows_.reserve((index + 1) * 2);
						}
						rows_.resize(index + 1);
						rows_[index] = data + use_size;
					}
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

			num_rows_ = kNumRowsIndex < rows_.size() ? atoi(rows_[kNumRowsIndex]) : 0;
			num_fields_ = kNumFieldIndex < rows_.size() ? atoi(rows_[kNumFieldIndex]) : 0;
		}

	private:
		WrapResult(const WrapResult&) = delete;
		WrapResult operator= (const WrapResult&) = delete;

	private:
		size_t num_rows_;
		size_t num_fields_;
		std::vector<const char*> rows_;
	};
}

#endif