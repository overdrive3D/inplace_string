template<class T, size_t N>
inplace_string<T, N>::inplace_string() noexcept
{
    buf[0] = '\0';
    buf[Capacity] = N;
}

template<class T, size_t N>
template<size_t M>
inline inplace_string<T, N>::inplace_string(const T (&str)[M]) noexcept:
    lit_str(str)
{
    len = M - 1;
    buf[Capacity] = Literal;
}

template<class T, size_t N>
inline inplace_string<T, N>::~inplace_string()
{
    if (spilled())
        delete[] str;
}

template<class T, size_t N>
inline const T *inplace_string<T, N>::c_str() const noexcept
{
    return inplace() ? buf : lit_str;
}

template<class T, size_t N>
inline size_t inplace_string<T, N>::length() const noexcept
{
    return inplace() ? (N - buf[N]) : len;
}

template<class T, size_t N>
inline size_t inplace_string<T, N>::capacity() const noexcept
{
    return (buf[Capacity] <= N) ? buf[Capacity] : 0;
}

template<class T, size_t N>
inline bool inplace_string<T, N>::inplace() const noexcept
{
    return (buf[Capacity] <= N);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::spilled() const noexcept
{
    return (Spilled == buf[N]);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::literal() const noexcept
{
    return (Literal == buf[N]);
}

template<class T>
inline size_t string_length(const T* s) noexcept
{
    if constexpr (std::is_same_v<T, char>)
        return strlen(s);
    if constexpr (std::is_same_v<T, wchar_t>)
        return wcslen(s);
    return 0;
}

template<class T, size_t N>
inline inplace_string<T, N>& inplace_string<T, N>::operator=(const T *s) noexcept
{
    size_t len = string_length(s);
    if (len <= N)
    {
        memcpy(buf, s, (len + 1) * sizeof(T));
        buf[Capacity] = T(N - len);
    }
    return *this;
}
