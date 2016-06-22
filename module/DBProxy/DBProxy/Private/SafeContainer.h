#ifndef __SAFE_CONTAINER_H__
#define __SAFE_CONTAINER_H__

#include <map>
#include <list>
#include <unordered_map>

template <typename Value>
class SafeQueue
{
public:
	SafeQueue() = default;

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
	SafeQueue(const SafeQueue&) = delete;
	SafeQueue& operator= (const SafeQueue&) = delete;

private:
	mutable std::mutex mutex_;
	std::list<Value> container_;
};

template <typename Key, typename Value>
class SafeMap
{
public:
	SafeMap() = default;

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
	SafeMap(const SafeMap&) = delete;
	SafeMap& operator= (const SafeMap&) = delete;

private:
	mutable std::mutex mutex_;
	std::map<Key, Value> container_;
};

template <typename Key, typename Value>
class SafeMultimap
{
public:
	SafeMultimap() = default;

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
	SafeMultimap(const SafeMultimap&) = delete;
	SafeMultimap& operator= (const SafeMultimap&) = delete;

private:
	mutable std::mutex mutex_;
	std::unordered_multimap<Key, Value> container_;
};

#endif