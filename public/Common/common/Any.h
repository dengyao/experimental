#ifndef __ANY_H__
#define __ANY_H__

#include <memory>

class BadAnyCast : public std::bad_cast
{
public:
	virtual const char* what() const throw()
	{
		return "bad any cast";
	}
};

class Any
{
public:
	Any() {}

	explicit Any(const Any &item)
	{
		if (item.data_ != nullptr)
		{
			item.data_->CopyTo(data_);
		}
	}

	explicit Any(Any &&item)
	{
		data_ = std::move(item.data_);
	}

	template <typename T>
	explicit Any(T &item)
		: Any(const_cast<const T&>(item))
	{
	}

	template <typename T>
	explicit Any(const T &item)
	{
		data_ = std::make_unique<Derived<T>>(item);
	}

	template <typename T>
	explicit Any(T &&item)
	{
		data_ = std::make_unique<Derived<T>>(std::forward<T>(item));
	}

	Any& operator= (const Any &item)
	{
		if (item.data_ != nullptr)
		{
			item.data_->CopyTo(data_);
		}
		return *this;
	}

	Any& operator= (Any &&item)
	{
		data_ = std::move(item.data_);
		return *this;
	}

	template<typename T>
	Any& operator= (T &item)
	{
		return operator=(const_cast<const T&>(item));
	}

	template<typename T>
	Any& operator= (const T &item)
	{
		data_ = std::make_unique<Derived<T>>(item);
		return *this;
	}

	template<typename T>
	Any& operator= (T &&item)
	{
		data_ = std::make_unique<Derived<T>>(std::forward<T>(item));
		return *this;
	}

	void Clear()
	{
		data_.reset();
	}

	template <typename T>
	bool Contains() const
	{
		return dynamic_cast<Derived<T>*>(data_.get()) != nullptr;
	}

	bool IsEmpty() const
	{
		return data_.get() == nullptr;
	}

	template <typename T>
	T& CastTo()
	{
		Derived<T> *derived = dynamic_cast<Derived<T>*>(data_.get());
		if (derived == nullptr)
		{
			throw BadAnyCast();
		}
		return derived->item;
	}

	template <typename T>
	const T& CastTo() const
	{
		Derived<T> *derived = dynamic_cast<Derived<T>*>(data_.get());
		if (derived == nullptr)
		{
			throw BadAnyCast();
		}
		return derived->item;
	}

	template <typename T>
	T& Get()
	{
		Derived<T> *derived = dynamic_cast<Derived<T>*>(data_.get());
		if (derived == nullptr)
		{
			data_ = std::make_unique<Derived<T>>();
		}
		return derived->item;
	}

	void Swap(Any &item)
	{
		data_.swap(item.data_);
	}

private:
	struct Base
	{
		virtual ~Base() {}
		virtual void CopyTo(std::unique_ptr<Base> &dest) const = 0;
	};

	template <typename T>
	struct Derived : public Base
	{
		T item;
		Derived() {}
		Derived(const T &val): item(val) {}
		Derived(T &&val) : item(std::forward<T>(val)) {}

		virtual void CopyTo(std::unique_ptr<Base> &dest) const
		{
			dest = std::make_unique<Derived<T>>(item);
		}
	};

	std::unique_ptr<Base> data_;
};

template <typename T> T& AnyCast(Any &a)
{
	return a.CastTo<T>();
}

template <typename T> const T& AnyCast(const Any &a)
{
	return a.CastTo<T>();
}

#endif