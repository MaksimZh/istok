// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <type_traits>
#include <mutex>

namespace Istok::Tools {

template<typename T, typename = void>
struct Hash {
    using type = std::hash<T>;
};

template<typename T>
struct Hash<T, std::void_t<typename T::Hasher>> {
    using type = typename T::Hasher;
};

template<typename T>
using hash = typename Hash<T>::type;


/**
 * @brief Auxiliary class for retrieving implicitly created instances
 * 
 * @param T Type of the instance to handle
 */
template <typename T>
class InstanceGetter {
public:
    /**
     * @brief Construct new instance
     * 
     * If previously created pointer is still not released
     * the constructor waits until it is.
     * Note that attempt to construct second instance in the same thread
     * without releasing the first one will result in deadlock.
     * 
     * @param self The pointer to store for later usage
     */
    InstanceGetter(T* self) : self(self) {
        std::unique_lock lock(mut);
        cv.wait(lock, [] { return instance == nullptr; });
        instance = self;
    }

    /**
     * @brief Destroy the instance
     * 
     * Releases the pointer it is still waiting.
     */
    ~InstanceGetter() {
        std::lock_guard lock(mut);
        if (instance == self) {
            instance = nullptr;
            cv.notify_all();
        }
    }

    /**
     * @brief Get pointer to the last created instance
     * 
     * @return T*
     */
    static T* release() {
        std::lock_guard lock(mut);
        T* tmp = instance;
        instance = nullptr;
        cv.notify_all();
        return tmp;
    }

private:
    T* self;
    static T* instance;
    static std::mutex mut;
    static std::condition_variable cv;
};

template <typename T>
T* InstanceGetter<T>::instance;

template <typename T>
std::mutex InstanceGetter<T>::mut;

template <typename T>
std::condition_variable InstanceGetter<T>::cv;


} // namespace Istok::Tools
