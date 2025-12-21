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
        free(str);
}

template<class T, size_t N>
inline const T *inplace_string<T, N>::c_str() const noexcept
{
    return inplace() ? buf : lit_str;
}

template<class T, size_t N>
inline size_t inplace_string<T, N>::length() const noexcept
{
    return inplace() ? (N - buf[Capacity]) : len;
}

template<class T, size_t N>
inline size_t inplace_string<T, N>::capacity() const noexcept
{
    return inplace() ? buf[Capacity] : cap;
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
inline inplace_string<T, N>& inplace_string<T, N>::operator+=(T ch) noexcept
{
    T& capacity = buf[Capacity];
    if (capacity && (capacity <= N))
    {
        T len = N - capacity--;
        buf[len] = ch;
        buf[len + 1] = '\0';
    }
    else
    {
        if (!spilled())
            spill(buf, N);
        append(ch);
    }
    return *this;
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator<(const inplace_string& s) const noexcept
{
    size_t len1 = length(), len2 = s.length();
    size_t len = std::min(len1, len2);
    int cmp = string_compare(c_str(), s.c_str(), len);
    return cmp ? cmp < 0 : len1 < len2;
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator<(const T *s) const noexcept
{
    size_t len1 = length(), len2 = string_length(s);
    size_t len = std::min(len1, len2);
    int cmp = string_compare(c_str(), s, len);
    return cmp ? cmp < 0 : len1 < len2;
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator<=(const inplace_string& s) const noexcept
{
    return !(s < *this);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator<=(const T *s) const noexcept
{
    return !(s < *this);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator>(const inplace_string& s) const noexcept
{
    return (s < *this);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator>(const T *s) const noexcept
{
    return (s < *this);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator>=(const inplace_string& s) const noexcept
{
    return !(*this < s);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator>=(const T *s) const noexcept
{
    return !(*this < s);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator==(const inplace_string& s) const noexcept
{
    size_t len = length();
    if (len != s.length())
        return false;
    return (0 == string_compare(c_str(), s.c_str(), len));
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator==(const T *s) const noexcept
{
    return (0 == string_compare(c_str(), s));
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator!=(const inplace_string& s) const noexcept
{
    return !(*this == s);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator!=(const T *s) const noexcept
{
    return !(*this == s);
}

template<class T, size_t N>
inline void inplace_string<T, N>::append(T ch) noexcept
{
    if (!cap) grow();
    str[len++] = ch;
    str[len] = '\0';
    --cap;
}

template<class T, size_t N>
inline void inplace_string<T, N>::spill(const T *src, size_t length) noexcept
{
    assert(inplace());
    assert(src);
    assert(length);
    const size_t count = length << 1;
    T *dst = (T *)malloc(count * sizeof(T));
    if (dst)
    {
        memcpy(dst, src, (length + 1) * sizeof(T)); // including '\0'
        str = dst;
        len = uint16_t(length);
        cap = uint16_t(count - length - 1);
        buf[Capacity] = Spilled;
    }
}

template<class T, size_t N>
inline void inplace_string<T, N>::grow() noexcept
{
    assert(spilled());
    assert(len);
    const size_t count = len << 1;
    T *grown = (T *)realloc(str, count * sizeof(T));
    if (grown)
    {
        str = grown;
        cap = uint16_t(count - len - 1);
    }
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
