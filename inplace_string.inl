template<class T, size_t N>
inplace_string<T, N>::inplace_string() noexcept:
    lit_str(nullptr)
{
    reset();
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
    return insitu() ? buf : lit_str;
}

template<class T, size_t N>
inline size_t inplace_string<T, N>::length() const noexcept
{
    return insitu() ? (N - buf[Capacity]) : len;
}

template<class T, size_t N>
inline size_t inplace_string<T, N>::capacity() const noexcept
{
    return insitu() ? buf[Capacity] : cap;
}

template<class T, size_t N>
inline bool inplace_string<T, N>::empty() const noexcept
{
    return (N == buf[Capacity]) && (0 == len);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::insitu() const noexcept
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
inline size_t inplace_string<T, N>::byte_size() const noexcept
{
    return (length() + 1) * sizeof(T);
}

template<class T, size_t N>
inline T inplace_string<T, N>::front() const noexcept
{
    const T *ch = element(0);
    assert(ch);
    return ch ? *ch : T(0);
}

template<class T, size_t N>
inline T& inplace_string<T, N>::front() noexcept
{
    T *ch = element(0);
    assert(ch);
    return *ch;
}

template<class T, size_t N>
inline T inplace_string<T, N>::back() const noexcept
{
    const T *ch = element(length() - 1);
    assert(ch);
    return ch ? *ch : T(0);
}

template<class T, size_t N>
inline T& inplace_string<T, N>::back() noexcept
{
    T *ch = element(length() - 1);
    assert(ch);
    return *ch;
}

template<class T, size_t N>
inline T *inplace_string<T, N>::begin() noexcept
{
    if (empty())
        return nullptr;
    return element(0);
}

template<class T, size_t N>
inline T *inplace_string<T, N>::end() noexcept
{
    if (empty())
        return nullptr;
    T *it = element(length());
    assert(it);
    assert('\0' == *it);
    return it;
}

template<class T, size_t N>
inline const T *inplace_string<T, N>::begin() const noexcept
{
    if (empty())
        return nullptr;
    return element(0);
}

template<class T, size_t N>
inline const T *inplace_string<T, N>::end() const noexcept
{
    const T *it = element(length());
    assert(it);
    assert('\0' == *it);
    return it;
}

template<class T, size_t N>
inline const T *inplace_string<T, N>::cbegin() const noexcept
{
    return begin();
}

template<class T, size_t N>
inline const T *inplace_string<T, N>::cend() const noexcept
{
    return end();
}

template<class T, size_t N>
inline inplace_string<T, N>& inplace_string<T, N>::operator=(const T *s) noexcept
{
    size_t length = string_length(s);
    if (length <= N)
    {
        memcpy(buf, s, (length + 1) * sizeof(T));
        buf[Capacity] = T(N - length);
    }
    else
        spill(s, length);
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
inline T *inplace_string<T, N>::element(size_t index) noexcept
{
    assert(!literal());
    assert(index <= length());
    if (insitu())
        return &buf[index];
    if (literal())
        return nullptr; // write denied
    return str ? str + index : nullptr;
}

template<class T, size_t N>
inline const T *inplace_string<T, N>::element(size_t index) const noexcept
{
    assert(index <= length());
    if (insitu())
        return &buf[index];
    return lit_str ? lit_str + index : nullptr;
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
    assert(insitu());
    assert(src);
    assert(length);
    const size_t count = length + (length >> 2);
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
template<size_t M>
inline void inplace_string<T, N>::move(inplace_string<T, M>& other) noexcept
{
    str = other.str;
    len = other.len;
    cap = other.cap;
    if (other.spilled())
        buf[Capacity] = Spilled;
    else if (other.literal())
        buf[Capacity] = Literal;
    other.str = nullptr;
    other.len = 0;
    other.cap = 0;
}

template<class T, size_t N>
inline void inplace_string<T, N>::reset() noexcept
{
    buf[0] = '\0';
    buf[Capacity] = N;
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
