#pragma once
#include <cstddef>

template <class T>
struct PoolAllocator
{
    typedef T value_type;
    PoolAllocator(char** place, char* end);
    template <class U> PoolAllocator(const PoolAllocator<U>& other);
    T* allocate(std::size_t n);
    void deallocate(T* p, std::size_t n);
};

template <class T, class U>
bool operator==(const PoolAllocator<T>&, const PoolAllocator<U>&);

template <class T, class U>
bool operator!=(const PoolAllocator<T>&, const PoolAllocator<U>&);
