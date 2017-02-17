#pragma once
#include <map>
#include <memory>

/// Manages resources of type T with identifiers of type ID.
template <class T, class ID>
class ResourceManager
{
public:
    using L = std::function(T(const ID&));
    ResourceManager(L loader): load(loader) {};

    std::shared_ptr<T> aquire(const ID& id)
    {
        if (auto cached_instance = cache[id].lock())
        {
            return cached_instance;
        }
        if (auto instance = load(id))
        {
            cache[id] = instance;
            return instance;
        }
        return nullptr;
    }

private:
    L load;
    std::map<D, std::weak_ptr<T>> cache;
}
