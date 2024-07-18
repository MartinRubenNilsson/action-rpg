#pragma once
#include "handle.h"

template <typename T>
class Pool
{
public:
	Handle<T> emplace(const T& t);
	T* get(Handle<T> handle);
	void free(Handle<T> handle);

private:
	std::vector<T> _data;
	std::vector<uint16_t> _generations; //Same size as _data.
	std::vector<uint16_t> _freelist; // Stores the indices of previously freed slots.
};

template<typename T>
inline Handle<T> Pool<T>::emplace(const T& t)
{
	Handle<T> handle;
	if (_freelist.empty()) {
		handle.index = (uint16_t)_data.size();
		handle.generation = 1;  // Valid generations start at 1.
		_generations.push_back(handle.generation);
		_data.emplace_back(t);
	} else {
		handle.index = _freelist.back();
		_freelist.pop_back();
		handle.generation = _generations[handle.index];
		_data[handle.index] = t;
	}
	return handle;
}

template<typename T>
inline T* Pool<T>::get(Handle<T> handle)
{
	if (handle.index >= _data.size()) return nullptr;
	if (handle.generation != _generations[handle.index]) return nullptr;
	return &_data[handle.index];
}

template<typename T>
inline void Pool<T>::free(Handle<T> handle)
{
	if (handle.index >= _data.size()) return;
	if (handle.generation != _generations[handle.index]) return;
	_generations[handle.index]++;
	_freelist.push_back(handle.index);
}
