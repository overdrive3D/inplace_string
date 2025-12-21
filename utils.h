#pragma once

template<class T>
inline size_t string_length(const T* s) noexcept
{
    assert(s);
    if constexpr (std::is_same_v<T, char>)
        return strlen(s);
    else if constexpr (std::is_same_v<T, wchar_t>)
        return wcslen(s);
    return 0;
}

template<class T>
inline int string_compare(const T* lhs, const T* rhs) noexcept
{
    assert(lhs);
    assert(rhs);
    if constexpr (std::is_same_v<T, char>)
        return strcmp(lhs, rhs);
    else if constexpr (std::is_same_v<T, wchar_t>)
        return wcscmp(lhs, rhs);
    return 0;
}

template<class T>
inline int string_compare(const T* lhs, const T* rhs, size_t count) noexcept
{
    assert(lhs);
    assert(rhs);
    assert(count);
    if constexpr (std::is_same_v<T, char>)
        return memcmp(lhs, rhs, count);
    else if constexpr (std::is_same_v<T, wchar_t>)
        return wmemcmp(lhs, rhs, count);
    return 0;
}
