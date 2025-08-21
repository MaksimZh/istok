// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once

#include <type_traits>

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

} // namespace Istok::Tools
