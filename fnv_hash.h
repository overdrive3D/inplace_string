#pragma once
#include <cstddef>
#include <cstdint>

template<class T>
constexpr uint32_t fnv1(const T array[], size_t N) noexcept
{
    constexpr uint32_t Basis = 0x811c9dc5;
    constexpr uint32_t Prime = 0x01000193;
    uint32_t hash = Basis;
    for (size_t i = 0; i < N; ++i)
        hash = (hash * Prime) ^ array[i];
    return hash;
}

template<class T, size_t N>
constexpr uint32_t fnv1(const T (&array)[N]) noexcept
{
    return fnv1(array, N);
}

template<class T>
constexpr uint32_t fnv1a(const T array[], size_t N) noexcept
{
    constexpr uint32_t Basis = 0x811c9dc5;
    constexpr uint32_t Prime = 0x01000193;
    uint32_t hash = Basis;
    for (size_t i = 0; i < N; ++i)
        hash = (hash ^ array[i]) * Prime;
    return hash;
}

template<class T, size_t N>
constexpr uint32_t fnv1a(const T (&array)[N]) noexcept
{
    return fnv1a(array, N);
}
