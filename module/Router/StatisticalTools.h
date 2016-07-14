#ifndef __STATISTICAL_TOOLS_H__
#define __STATISTICAL_TOOLS_H__

#include <common/Singleton.h>

class StatisticalTools : public Singleton<StatisticalTools>
{
	struct Data 
	{
		uint32_t up_volume;
		uint32_t down_volume;

		Data()
		{
			Zero();
		}

		void Zero()
		{
			up_volume = 0;
			down_volume = 0;
		}
	};

public:
	void Flush()
	{
		last_data_ = current_data_;
		current_data_.Zero();
	}

public:
	// 每秒上行流量
	uint32_t UpVolume() const
	{
		return last_data_.up_volume;
	}

	// 每秒下行流量
	uint32_t DownVolume() const
	{
		return last_data_.down_volume;
	}

public:
	// 累积每秒上行流量
	void AccumulateUpVolume(uint32_t value)
	{
		current_data_.up_volume += value;
	}

	// 累积每秒下行流量
	void AccumulateDownVolume(uint32_t value)
	{
		current_data_.down_volume += value;
	}

private:
	StatisticalTools() = default;
	~StatisticalTools() = default;
	friend class Singleton<StatisticalTools>;

private:
	Data last_data_;
	Data current_data_;
};

#endif
