#ifndef __ID_GENERATOR_H__
#define __ID_GENERATOR_H__

#include <vector>
#include <cstdint>

namespace network
{
	class IDGenerator
	{
		IDGenerator(const IDGenerator&) = delete;
		IDGenerator& operator= (const IDGenerator&) = delete;

	public:
		static const uint32_t kInvalidID = 0;
		static const uint32_t kDefaultThreshold = 8192;

	public:
		explicit IDGenerator(uint32_t threshold = kDefaultThreshold)
			: next_(kInvalidID), threshold_(threshold)
		{
		}

		~IDGenerator() = default;

		uint32_t Get()
		{
			if (pools_.size() >= threshold_)
			{
				uint32_t id = pools_.back();
				pools_.pop_back();
				return id;
			}
			else
			{
				return ++next_;
			}
		}

		void Put(uint32_t id)
		{
#ifdef DEBUG
			for (size_t i = 0; i < pools_.size(); ++i)
			{
				assert(pools_[i] != id);
			}
#endif // _DEBUG
			pools_.push_back(id);
		}

		void Erase(uint32_t id)
		{
			for (auto iter = pools_.begin(); iter != pools_.end(); ++iter)
			{
				if (*iter == id)
				{
					pools_.erase(iter);
					break;
				}				
			}		
		}

	private:
		uint32_t				next_;
		const size_t			threshold_;
		std::vector<uint32_t>	pools_;
	};
}

#endif