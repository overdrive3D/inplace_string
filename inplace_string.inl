template<class T, size_t N>
inline inplace_string<T, N>::inplace_string() noexcept:
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
    cap = 0;
    uid = Unhashed;
    buf[Capacity] = Literal;
}

template<class T, size_t N>
inline inplace_string<T, N>::inplace_string(inplace_string&& other) noexcept
{
    if (other.insitu()) [[likely]]
        memcpy(buf, other.buf, sizeof(buf));
    else
        move(other);
    other.reset();
}

template<class T, size_t N>
template<size_t M>
inline inplace_string<T, N>::inplace_string(inplace_string<T, M>&& other) noexcept
{
    const size_t length = other.length();
    if (length <= N) [[likely]]
    {
        copy_inplace(other.c_str(), length);
        if (other.spilled())
            free(other.str);
    }
    else if (other.insitu())
        spill(other.buf, length);
    else
        move(other);
    other.reset();
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
inline size_t inplace_string<T, N>::bytes_size() const noexcept
{
    return (length() + 1) * sizeof(T);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::empty() const noexcept
{
    return insitu() ? (N == buf[Capacity]) : (0 == len);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::insitu() const noexcept
{
    if constexpr (std::is_same_v<T, char> && std::is_signed_v<char>)
        return (buf[Capacity] >= 0);
    else
        return (buf[Capacity] < Literal);
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
inline T inplace_string<T, N>::front() const noexcept
{
    assert(!empty());
    return *begin();
}

template<class T, size_t N>
inline T& inplace_string<T, N>::front() noexcept
{
    assert(!empty());
    return *begin();
}

template<class T, size_t N>
inline T inplace_string<T, N>::back() const noexcept
{
    assert(!empty());
    return *(end() - 1);
}

template<class T, size_t N>
inline T& inplace_string<T, N>::back() noexcept
{
    assert(!empty());
    return *(end() - 1);
}

template<class T, size_t N>
inline T inplace_string<T, N>::at(size_t index) const noexcept
{
    assert(!empty());
    assert(index < length());
    return insitu() ? buf[index] : str[index];
}

template<class T, size_t N>
inline T& inplace_string<T, N>::at(size_t index) noexcept
{
    assert(!empty());
    assert(!literal());
    assert(index < length());
    bool sso = insitu();
    if (!sso) [[unlikely]]
        uid = Unhashed; // invalidate hash
    return sso ? buf[index] : str[index];
}

template<class T, size_t N>
inline T *inplace_string<T, N>::begin() noexcept
{
    assert(!literal());
    if (literal())
        return nullptr; // can't write
    bool sso = insitu();
    if (!sso) [[unlikely]]
        uid = Unhashed; // invalidate hash
    return sso ? buf : str;
}

template<class T, size_t N>
inline T *inplace_string<T, N>::end() noexcept
{
    assert(!literal());
    if (literal())
        return nullptr; // can't write
    bool sso = insitu();
    if (!sso) [[unlikely]]
        uid = Unhashed; // invalidate hash
    T *end = sso
        ? buf + (N - buf[Capacity])
        : str + len;
    assert('\0' == *end);
    return end;
}

template<class T, size_t N>
inline const T *inplace_string<T, N>::begin() const noexcept
{
    return insitu() ? buf : str;
}

template<class T, size_t N>
inline const T *inplace_string<T, N>::end() const noexcept
{
    const T *end = insitu()
        ? buf + (N - buf[Capacity])
        : str + len;
    assert('\0' == *end);
    return end;
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
inline void inplace_string<T, N>::push_back(T ch) noexcept
{
    assert(!literal());
    T& capacity = buf[Capacity];
    if (capacity > 0) [[likely]]
    {
        size_t len = N - capacity--;
        buf[len] = ch;
        buf[len + 1] = '\0';
    }
    else [[unlikely]]
    {
        if (!capacity)
            spill(buf, N);
        else if (!cap)
            grow();
        str[len++] = ch;
        str[len] = '\0';
        --cap;
        uid = Unhashed;
    }
}

template<class T, size_t N>
inline void inplace_string<T, N>::pop_back() noexcept
{
    assert(!empty());
    assert(!literal());
    T& capacity = buf[Capacity];
    if (capacity >= 0) [[likely]]
    {
        size_t len = N - capacity++;
        buf[len - 1] = '\0';
    }
    else [[unlikely]]
    {
        str[--len] = '\0';
        ++cap;
        uid = Unhashed;
    }
}

template<class T, size_t N>
inline inplace_string<T, N> inplace_string<T, N>::substr(size_t pos, size_t count) const noexcept
{
    size_t len = length();
    if (pos >= len)
        return inplace_string();
    count = std::min(count, len - pos);
    if (literal() && ('\0' == lit_str[pos + count]))
        return inplace_string(lit_str, pos, count);
    inplace_string sub;
    const T *first = insitu() ? (buf + pos) : (str + pos);
    if (count <= N) [[likely]]
        sub.copy_inplace(first, count);
    else [[unlikely]]
        sub.spill(first, count);
    return sub;
}

template<class T, size_t N>
template<size_t M>
inline inplace_string<T, N>& inplace_string<T, N>::replace(size_t pos, size_t count, const inplace_string<T, M>& other) noexcept
{
    assert(pos < length());
    if (pos >= length())
        return *this;
    assert(count <= other.length());
    bool buy = literal() || (pos + count > length());
    if (buy)
    {
        size_t length = pos + count, space;
        if (auto dst = buy_space(length, space))
        {
            memcpy(dst, this->c_str(), pos * sizeof(T));
            memcpy(dst + pos, other.c_str(), count * sizeof(T));
            dst[length] = '\0';
            if (spilled())
                free(str);
            str = dst;
            len = length;
            cap = space - length - 1;
            uid = Unhashed;
            buf[Capacity] = Spilled;
        }
    }
    else
    {
        // TODO
    }
    return *this;
}

template<class T, size_t N>
inline uint32_t inplace_string<T, N>::hash() const noexcept
{
    constexpr uint32_t OffsetBasis = 0x811c9dc5;
    constexpr uint32_t Prime = 0x01000193;
    const uint8_t *data = (const uint8_t *)c_str();
    uint32_t hash = OffsetBasis;
    for (size_t i = 0, size = length() * sizeof(T); i < size; ++i)
    {
        hash ^= data[i];
        hash *= Prime;
    }
    if (!insitu())
        uid = hash;
    return hash;
}

template<class T, size_t N>
inline bool inplace_string<T, N>::hashed() const noexcept
{
    return !insitu() && (uid != Unhashed);
}

template<class T, size_t N>
inline inplace_string<T, N>& inplace_string<T, N>::operator=(const inplace_string& s) noexcept
{
    if (s.literal())
    {
        ~inplace_string();
        lit_str = s.lit_str;
        len = s.len;
        cap = 0;
        buf[Capacity] = Literal;
    }
    else if (s.insitu())
    {
        ~inplace_string();
        copy_inplace(s.buf, s.length());
    }
    else // if (s.spilled())
    {
        size_t size = s.bytes_size();
        if (!spilled())
            copy_heap(s.str, s.len, size);
        else
        {
            if (len >= s.len)
            {
                memcpy(str, s.str, size);
                cap += (len - s.len);
                len = s.len;
            }
            else
            {
                // TODO: grow and copy
            }
        }
    }

    return *this;
}

template<class T, size_t N>
inline inplace_string<T, N>& inplace_string<T, N>::operator=(const T *s) noexcept
{
    size_t length = string_length(s);
    if (length <= N)
        copy_inplace(s, length);
    else
        spill(s, length);
    return *this;
}

template<class T, size_t N>
inline inplace_string<T, N>& inplace_string<T, N>::operator+=(T ch) noexcept
{
    push_back(ch);
    return *this;
}

template<class T, size_t N>
template<size_t M>
inline bool inplace_string<T, N>::operator<(const inplace_string<T, M>& s) const noexcept
{
    bool eq = hashed() && s.hashed() && (uid == s.uid);
    if (eq) [[unlikely]]
        return false;
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
template<size_t M>
inline bool inplace_string<T, N>::operator<=(const inplace_string<T, M>& s) const noexcept
{
    return !(s < *this);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator<=(const T *s) const noexcept
{
    return !(s < *this);
}

template<class T, size_t N>
template<size_t M>
inline bool inplace_string<T, N>::operator>(const inplace_string<T, M>& s) const noexcept
{
    return (s < *this);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator>(const T *s) const noexcept
{
    return (s < *this);
}

template<class T, size_t N>
template<size_t M>
inline bool inplace_string<T, N>::operator>=(const inplace_string<T, M>& s) const noexcept
{
    return !(*this < s);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator>=(const T *s) const noexcept
{
    return !(*this < s);
}

template<class T, size_t N>
template<size_t M>
inline bool inplace_string<T, N>::operator==(const inplace_string<T, M>& s) const noexcept
{
    size_t len = length();
    if (len != s.length()) [[likely]]
        return false;
    if (hashed() && s.hashed()) [[likely]]
        return (uid == s.uid);
    return (0 == string_compare(c_str(), s.c_str(), len));
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator==(const T *s) const noexcept
{
    return (0 == string_compare(c_str(), s));
}

template<class T, size_t N>
template<size_t M>
inline bool inplace_string<T, N>::operator!=(const inplace_string<T, M>& s) const noexcept
{
    return !(*this == s);
}

template<class T, size_t N>
inline bool inplace_string<T, N>::operator!=(const T *s) const noexcept
{
    return !(*this == s);
}

template<class T, size_t N>
inline T inplace_string<T, N>::operator[](size_t index) const noexcept
{
    assert(index < length());
    return insitu() ? buf[index] : str[index];
}

template<class T, size_t N>
inline inplace_string<T, N>::inplace_string(const T *str, size_t offset, size_t length) noexcept:
    lit_str(str + offset)
{
    len = length;
    cap = 0;
    uid = Unhashed;
    buf[Capacity] = Literal;
}

template<class T, size_t N>
inline void inplace_string<T, N>::copy_inplace(const T *c_str, size_t length) noexcept
{
    assert(!spilled()); // Don't overwrite heap pointer
    assert(length <= N);
    memcpy(buf, c_str, length * sizeof(T));
    buf[length] = '\0';
    buf[Capacity] = T(N - length);
}

template<class T, size_t N>
inline void inplace_string<T, N>::copy_heap(const T *src, size_t length, size_t size) noexcept
{
    void *dst = malloc(size);
    if (dst)
    {
        str = (T *)memcpy(dst, src, size); // including '\0'
        len = length;
        cap = 0;
        uid = Unhashed;
        buf[Capacity] = Spilled;
    }
}

template<class T, size_t N>
inline void inplace_string<T, N>::spill(const T *src, size_t length) noexcept
{
    assert(!spilled());
    assert(src);
    assert(length);
    size_t space;
    if (auto dst = buy_space(length, space))
    {   // Copy string including '\0'
        memcpy(dst, src, (length + 1) * sizeof(T));
        str = dst;
        len = length;
        cap = space - length - 1;
        uid = Unhashed;
        buf[Capacity] = Spilled;
    }
}

template<class T, size_t N>
inline void inplace_string<T, N>::grow() noexcept
{
    assert(spilled());
    assert(len);
    const size_t count = len << 1;
    void *grown = realloc(str, count * sizeof(T));
    if (grown)
    {
        str = (T *)grown;
        cap = count - len - 1;
    }
}

template<class T, size_t N>
inline T *inplace_string<T, N>::buy_space(size_t much, size_t& space) noexcept
{
    space = much + (much >> 1);
    const size_t size = space * sizeof(T);
    if (insitu() || literal())
        return (T *)malloc(size);
    else
        return (T *)realloc(str, size);
}

template<class T, size_t N>
template<size_t M>
inline void inplace_string<T, N>::move(inplace_string<T, M>& other) noexcept
{
    str = other.str;
    len = other.len;
    cap = other.cap;
    uid = other.uid;
    if (other.spilled())
        buf[Capacity] = Spilled;
    else if (other.literal())
        buf[Capacity] = Literal;
    other.str = nullptr;
    other.len = 0;
    other.cap = 0;
    other.uid = Unhashed;
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
    if (!str.empty())
        os << str.c_str();
    return os;
}
