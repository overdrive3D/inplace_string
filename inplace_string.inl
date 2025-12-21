template<class T, size_t N>
inplace_string<T, N>::inplace_string() noexcept:
    lit_str(nullptr)
{
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
inline bool inplace_string<T, N>::empty() const noexcept
{
    return (N == buf[Capacity]);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::inplace() const noexcept
{
    return (buf[Capacity] <= N);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::spilled() const noexcept
{
    return (Spilled == buf[Capacity]);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::literal() const noexcept
{
    return (Literal == buf[Capacity]);
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

template<class T, size_t N>
inline bool operator<(const T* lhs, const inplace_string<T, N>& rhs) noexcept
{
    size_t len1 = string_length(lhs), len2 = rhs.length();
    size_t len = std::min(len1, len2);
    int cmp = string_compare(lhs, rhs.c_str(), len);
    return cmp ? cmp < 0 : len1 < len2;
}

template<class T, size_t N>
inline std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, const inplace_string<T, N>& str)
{
    return os << str.c_str();
}
