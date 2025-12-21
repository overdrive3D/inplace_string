#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <type_traits>
#include <iostream>

#include "utils.h"

template<class T, size_t N = 15>
class alignas(16) inplace_string
{
    static_assert(std::is_same_v<T, char> ||
                  std::is_same_v<T, wchar_t>,
        "inplace string requires char or wchar_t type");
    static_assert(N >= 15, "inplace string string too short");
    static_assert(N < std::numeric_limits<T>::max() - 1, "inplace string string too long");
    static_assert(sizeof(T[N + 2]) >= sizeof(T*) + sizeof(uint16_t) + sizeof(uint8_t),
        "internal buffer too small for aliasing");

public:
    using type = T;
    using iterator = T*;
    using const_iterator = const T*;

    inplace_string() noexcept;
    template<size_t M>
    inplace_string(const T (&str)[M]) noexcept;
    ~inplace_string();
    const T *c_str() const noexcept;
    size_t length() const noexcept;
    size_t capacity() const noexcept;
    bool empty() const noexcept;
    bool inplace() const noexcept;
    bool spilled() const noexcept;
    bool literal() const noexcept;

    inplace_string& operator=(const T*) noexcept;

private:
    static constexpr size_t Capacity = N;
    static constexpr T Spilled = std::numeric_limits<T>::max();
    static constexpr T Literal = std::numeric_limits<T>::max() - 1;

    union
    {
        T buf[N + 1];
        struct { T *str; uint16_t len; };
        struct { const T *const lit_str; };
    };
};

static_assert(alignof(inplace_string<char>) == 16,
    "invalid alignment of inplace_string");
static_assert(sizeof(inplace_string<char>) == 16,
    "invalid default size of inplace_string");

#include "inplace_string.inl"
