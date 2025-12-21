#pragma once
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <algorithm>
#include <type_traits>
#include <iostream>

#include "utils.h"

template<class T, size_t N = 15>
class alignas(16) inplace_string
{
    static_assert(std::is_same_v<T, char> ||
                  std::is_same_v<T, wchar_t>,
        "inplace string requires char or wchar_t type");
    static_assert(N >= 15, "inplace string too short");
    static_assert(N < std::numeric_limits<T>::max() - 1, "inplace string too long");
    static_assert(sizeof(T[N + 1]) >= sizeof(T*) + sizeof(uint32_t),
        "sso buffer too small for aliasing");

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
    bool insitu() const noexcept;
    bool spilled() const noexcept;
    bool literal() const noexcept;
    size_t byte_size() const noexcept;
    T front() const noexcept;
    T& front() noexcept;
    T back() const noexcept;
    T& back() noexcept;
    T *begin() noexcept;
    T *end() noexcept;
    const T *begin() const noexcept;
    const T *end() const noexcept;
    const T *cbegin() const noexcept;
    const T *cend() const noexcept;

    inplace_string& operator=(const T*) noexcept;

    inplace_string& operator+=(T) noexcept;
    bool operator<(const inplace_string&) const noexcept;
    bool operator<(const T*) const noexcept;
    bool operator<=(const inplace_string&) const noexcept;
    bool operator<=(const T*) const noexcept;
    bool operator>(const inplace_string&) const noexcept;
    bool operator>(const T*) const noexcept;
    bool operator>=(const inplace_string&) const noexcept;
    bool operator>=(const T*) const noexcept;
    bool operator==(const inplace_string&) const noexcept;
    bool operator==(const T*) const noexcept;
    bool operator!=(const inplace_string&) const noexcept;
    bool operator!=(const T*) const noexcept;

private:
    static constexpr size_t Capacity = N;
    static constexpr T Spilled = -1;
    static constexpr T Literal = -2;

    T *element(size_t index) noexcept;
    const T *element(size_t index) const noexcept;
    void append(T ch) noexcept;
    void spill(const T *s, size_t length) noexcept;
    void grow() noexcept;
    void reset() noexcept;

    union
    {
        T buf[N + 1];
        struct { T *str; uint16_t len; uint16_t cap; };
        struct { const T *const lit_str; };
    };
};

static_assert(alignof(inplace_string<char>) == 16,
    "invalid alignment of inplace_string");
static_assert(sizeof(inplace_string<char>) == 16,
    "invalid default size of inplace_string");

template<size_t N = 15> using string = inplace_string<char, N>;
template<size_t N = 15> using wstring = inplace_string<wchar_t, N>;

template<class T, size_t N>
bool operator<(const T*, const inplace_string<T, N>&) noexcept;

#include "inplace_string.inl"
