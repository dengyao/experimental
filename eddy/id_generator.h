#pragma once

#include <vector>
#include <cstdint>


namespace eddy
{
	class IDGenerator
	{
	public:
		static const uint32_t kInvalidID = 0;

	public:
		explicit IDGenerator(uint32_t threshold = 8192)
			: next_(kInvalidID), threshold_(threshold) {}

		bool get(uint32_t &id)
		{
			if (pool_.size() > threshold_)
			{
				id = pool_.back();
				pool_.pop_back();
				return true;
			}
			else if (next_ <= threshold_)
			{
				id = ++next_;
				return true;
			}
			return false;
		}

		void put(uint32_t id)
		{
			pool_.push_back(id);
		}

	private:
		uint32_t				next_;
		const size_t			threshold_;
		std::vector<uint32_t>	pool_;
	};
}