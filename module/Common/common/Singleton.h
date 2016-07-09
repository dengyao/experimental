#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include <list>
#include <algorithm>

class SingletonBase
{
	class InstanceTable : public std::list < SingletonBase * >
	{
	public:
		InstanceTable()
			: is_cleared_(false)
		{
		}

		~InstanceTable()
		{
			is_cleared_ = true;
			for (iterator itr = begin(); itr != end(); ++itr)
			{
				delete *itr;
			}
		}

	public:
		bool is_cleared_;
	};

protected:
	SingletonBase()
	{
		s_instance_table_.push_back(this);
	}

	virtual ~SingletonBase()
	{
		if (!s_instance_table_.is_cleared_)
		{
			InstanceTable::iterator found = std::find(s_instance_table_.begin(), s_instance_table_.end(), this);
			if (found != s_instance_table_.end())
			{
				s_instance_table_.erase(found);
			}
		}
	}

private:
	static InstanceTable s_instance_table_;
};

template <typename T>
class Singleton : public SingletonBase
{
public:
	static T* GetInstance()
	{
		if (s_singleton_ == NULL)
		{
			s_singleton_ = new T();
		}
		return s_singleton_;
	}

	static void DestroyInstance()
	{
		if (s_singleton_)
		{
			delete s_singleton_;
		}
	}

protected:
	Singleton() {}

	virtual ~Singleton()
	{
		s_singleton_ = NULL;
	}

private:
	Singleton(const Singleton &that) {}
	Singleton& operator=(const Singleton &that) {}

private:
	static T* s_singleton_;
};

template<typename T> T* Singleton<T>::s_singleton_ = NULL;

#define SINGLETON(_class_name_)					\
	private:									\
		_class_name_();							\
		~_class_name_();						\
		friend class Singleton<_class_name_>

#endif