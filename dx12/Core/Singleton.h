#pragma once


template <typename T>
class Singleton {

public:
	Singleton(const Singleton&) = delete;
	Singleton(Singleton&&) = delete;

	virtual bool init() = 0;
	virtual void shutdown() = 0;

	inline static T& get() {
		static T instance;
		return instance;
	}

protected:
	Singleton() = default;


};