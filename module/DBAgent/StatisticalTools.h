#ifndef __STATISTICAL_TOOLS_H__
#define __STATISTICAL_TOOLS_H__

#include <common/Singleton.h>

class StatisticalTools : public Singleton<StatisticalTools>
{
	struct Data 
	{
		uint32_t up_volume;
		uint32_t down_volume;
		uint32_t handle_call_count;
		uint32_t handle_select_count;
		uint32_t handle_insert_count;
		uint32_t handle_update_count;
		uint32_t handle_delete_count;

		Data()
		{
			Zero();
		}

		void Zero()
		{
			up_volume = 0;
			down_volume = 0;
			handle_call_count = 0;
			handle_select_count = 0;
			handle_insert_count = 0;
			handle_update_count = 0;
			handle_delete_count = 0;
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

	// 每秒处理调用数量
	uint32_t HandleCallCount() const
	{
		return last_data_.handle_call_count;
	}

	// 每秒处理查询数量
	uint32_t HandleSelectCount() const
	{
		return last_data_.handle_select_count;
	}

	// 每秒处理插入数量
	uint32_t HandleInsertCount() const
	{
		return last_data_.handle_insert_count;
	}

	// 每秒处理更新数量
	uint32_t HandleUpdateCount() const
	{
		return last_data_.handle_update_count;
	}

	// 每秒处理删除数量
	uint32_t HandleDeleteCount() const
	{
		return last_data_.handle_delete_count;
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

	// 累积每秒处理调用数量
	void AccumulateHandleCallCount(uint32_t value)
	{
		current_data_.handle_call_count += value;
	}

	// 累积每秒处理查询数量
	void AccumulateHandleSelectCount(uint32_t value)
	{
		current_data_.handle_select_count += value;
	}

	// 累积每秒处理插入数量
	void AccumulateHandleInsertCount(uint32_t value)
	{
		current_data_.handle_insert_count += value;
	}

	// 累积每秒处理更新数量
	void AccumulateHandleUpdateCount(uint32_t value)
	{
		current_data_.handle_update_count += value;
	}

	// 累积每秒处理删除数量
	void AccumulateHandleDeleteCount(uint32_t value)
	{
		current_data_.handle_delete_count += value;
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
