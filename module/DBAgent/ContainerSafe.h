#ifndef __CONTAINER_SAFE_H__
#define __CONTAINER_SAFE_H__

#include <map>
#include <list>
#include <deque>
#include <vector>
#include <mutex>

template <typename Value>
class QueueSafe
{
public:
	QueueSafe() = default;

	bool Take(Value &value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (container_.empty())
		{
			return false;
		}
		value = std::move(container_.front());
		container_.pop_front();
		return true;
	}

	size_t TakeAll(std::vector<Value> &vec)
	{
		size_t size = 0;
		std::lock_guard<std::mutex> lock(mutex_);
		size = container_.size();
		vec.reserve(vec.size() + size);
		while (!container_.empty())
		{
			vec.push_back(std::move(container_.front()));
			container_.pop_front();
		}
		return size;
	}

	void Append(const Value &value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		container_.push_back(value);
	}

	void Append(Value &&value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		container_.push_back(std::forward<Value>(value));
	}

	size_t Size() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return container_.size();
	}

private:
	QueueSafe(const QueueSafe&) = delete;
	QueueSafe& operator= (const QueueSafe&) = delete;

private:
	mutable std::mutex mutex_;
	std::deque<Value> container_;
};

template <typename Key, typename Value>
class MapSafe
{
public:
	MapSafe() = default;

	bool IsExist(const Key &key) const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return container_.find(key) != container_.end();
	}

	bool Get(const Key &key, Value *&value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto found = container_.find(key);
		if (found == container_.end())
		{
			return false;
		}
		value = &found->second;
		return true;
	}

	bool Take(const Key &key, Value &value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto found = container_.find(key);
		if (found == container_.end())
		{
			return false;
		}
		value = std::move(found->second);
		container_.erase(found);
		return true;
	}

	void Append(const Key &key)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		container_[key];
	}

	void Append(const Key &key, const Value &value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		container_.insert(std::make_pair(key, value));
	}

	void Append(const Key &key, Value &&value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		container_.insert(std::make_pair(key, std::forward<Value>(value)));
	}

	size_t Size() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return container_.size();
	}

private:
	MapSafe(const MapSafe&) = delete;
	MapSafe& operator= (const MapSafe&) = delete;

private:
	mutable std::mutex mutex_;
	std::map<Key, Value> container_;
};

template <typename Key, typename Value>
class MultimapSafe
{
public:
	MultimapSafe() = default;

	bool Take(const Key &key, Value &value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto range = container_.equal_range(key);
		if (range.first == range.second)
		{
			return false;
		}
		value = std::move(range.first->second);
		container_.erase(range.first);
		return true;
	}

	void Append(const Key &key, const Value &value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		container_.insert(std::make_pair(key, value));
	}

	void Append(const Key &key, Value &&value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		container_.insert(std::make_pair(key, std::forward<Value>(value)));
	}

	size_t Size() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return container_.size();
	}

private:
	MultimapSafe(const MultimapSafe&) = delete;
	MultimapSafe& operator= (const MultimapSafe&) = delete;

private:
	mutable std::mutex mutex_;
	std::multimap<Key, Value> container_;
};

#endif