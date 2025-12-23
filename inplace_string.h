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
    static constexpr size_t BufSize = sizeof(T[N + 1]);
    static_assert(BufSize > sizeof(T*) + sizeof(uint32_t),
        "sso buffer too small for aliasing");
    template<class, size_t>
    friend class inplace_string;

public:
    using type = T;
    using iterator = T*;
    using const_iterator = const T*;
    static constexpr size_t npos = size_t(-1);

    inplace_string() noexcept;
    template<size_t M>
    inplace_string(const T (&str)[M]) noexcept;
    inplace_string(inplace_string&&) noexcept;
    template<size_t M>
    inplace_string(inplace_string<T, M>&&) noexcept;
    ~inplace_string();
    const T *c_str() const noexcept;
    size_t length() const noexcept;
    size_t capacity() const noexcept;
    size_t bytes_size() const noexcept;
    bool empty() const noexcept;
    bool insitu() const noexcept;
    bool spilled() const noexcept;
    bool literal() const noexcept;
    T front() const noexcept;
    T& front() noexcept;
    T back() const noexcept;
    T& back() noexcept;
    T at(size_t index) const noexcept;
    T& at(size_t index) noexcept;
    T *begin() noexcept;
    T *end() noexcept;
    const T *begin() const noexcept;
    const T *end() const noexcept;
    const T *cbegin() const noexcept;
    const T *cend() const noexcept;
    void push_back(T ch) noexcept;
    void pop_back() noexcept;
    inplace_string substr(size_t pos, size_t count = npos) const noexcept;
    uint32_t hash() const noexcept;
    bool hashed() const noexcept;

    inplace_string& operator=(const inplace_string&) noexcept;
    inplace_string& operator=(const T*) noexcept;

    inplace_string& operator+=(T) noexcept;
    template<size_t M>
    bool operator<(const inplace_string<T, M>&) const noexcept;
    bool operator<(const T*) const noexcept;
    template<size_t M>
    bool operator<=(const inplace_string<T, M>&) const noexcept;
    bool operator<=(const T*) const noexcept;
    template<size_t M>
    bool operator>(const inplace_string<T, M>&) const noexcept;
    bool operator>(const T*) const noexcept;
    template<size_t M>
    bool operator>=(const inplace_string<T, M>&) const noexcept;
    bool operator>=(const T*) const noexcept;
    template<size_t M>
    bool operator==(const inplace_string<T, M>&) const noexcept;
    bool operator==(const T*) const noexcept;
    template<size_t M>
    bool operator!=(const inplace_string<T, M>&) const noexcept;
    bool operator!=(const T*) const noexcept;
    T operator[](size_t) const noexcept;

private:
    static constexpr size_t Capacity = N;
    static constexpr T Spilled = -1;
    static constexpr T Literal = -2;
    static constexpr uint32_t Unhashed = std::numeric_limits<uint32_t>::max();

    inplace_string(const T *lit_str, size_t offset, size_t length) noexcept;
    void copy_inplace(const T *c_str, size_t length) noexcept;
    void copy_heap(const T *src, size_t length, size_t size) noexcept;
    void spill(const T *s, size_t length) noexcept;
    void grow() noexcept;
    template<size_t M>
    void move(inplace_string<T, M>&) noexcept;
    T *element(size_t index) noexcept;
    const T *element(size_t index) const noexcept;
    void reset() noexcept;

    union
    {
        T buf[N + 1];
        struct { const T *const lit_str; };
        struct {
            T *str;
            struct {
                uint64_t len: 12;
                uint64_t cap: 12;
                mutable uint64_t uid: 32;
            };
        };
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
