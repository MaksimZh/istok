// Copyright 2025 Maksim Sergeevich Zholudev. All rights reserved
#pragma once


namespace Istok::Tools {

template<typename T>
concept HasHasher = requires { typename T::Hasher; };

template<typename T>
using hash = std::conditional_t<
    HasHasher<T>,
    typename T::Hasher,
    std::hash<T>
>;

} // namespace Istok::Tools
