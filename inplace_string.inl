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
    init(M - 1, 0, Literal);
}

template<class T, size_t N>
inline inplace_string<T, N>::inplace_string(const literal_string<T>& lit) noexcept:
    lit_str(lit.c_str())
{
    init(lit.length(), 0, Literal, lit.hash());
}

template<class T, size_t N>
inline inplace_string<T, N>::inplace_string(const inplace_string& str) noexcept:
    lit_str(str.lit_str)
{
    if (str.literal())
        init(str.len, str.cap, Literal, str.uid);
    else
        copy_ctor(str);
}

template<class T, size_t N>
template<size_t M>
inline inplace_string<T, N>::inplace_string(const inplace_string<T, M>& str) noexcept:
    lit_str(str.lit_str)
{
    if (str.literal())
        init(str.len, str.cap, Literal, str.uid);
    else
        copy_ctor(str);
}

template<class T, size_t N>
inline inplace_string<T, N>::inplace_string(inplace_string&& str) noexcept:
    lit_str(str.lit_str)
{
    if (str.insitu()) [[likely]]
        copy_ctor(str);
    else [[unlikely]]
        init(str.len, str.cap, /* flag */ str.buf[N], str.uid);
    str.reset();
}

template<class T, size_t N>
template<size_t M>
inline inplace_string<T, N>::inplace_string(inplace_string<T, M>&& str) noexcept:
    lit_str(str.lit_str)
{
    if (str.insitu()) [[likely]]
        copy_ctor(str);
    else [[unlikely]]
        init(str.len, str.cap, /* flag */ str.buf[M], str.uid);
    str.reset();
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
        buf[len + 1] = T('\0');
    }
    else [[unlikely]]
    {
        if (!capacity)
            spill(buf, N);
        else if (!cap)
            grow();
        str[len++] = ch;
        str[len] = T('\0');
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
        buf[len - 1] = T('\0');
    }
    else [[unlikely]]
    {
        str[--len] = T('\0');
        ++cap;
        uid = Unhashed;
    }
}

template<class T, size_t N>
inline size_t inplace_string<T, N>::find(T ch, size_t pos /* 0 */) const noexcept
{
    size_t len = length();
    assert(pos < len);
    if (pos >= len)
        return npos;
    const T *begin = c_str() + pos;
    const void *found;
    if constexpr (std::is_same_v<T, char>)
        found = memchr(begin, ch, len - pos);
    else
        found = wmemchr(begin, ch, len - pos);
    if (!found)
        return npos;
    return (const T*)found - begin + pos;
}

template<class T, size_t N>
inline size_t inplace_string<T, N>::find(const T *substr, size_t pos /* 0 */) const noexcept
{
    assert(substr);
    size_t len = length();
    assert(pos < len);
    if (pos >= len)
        return npos;
    const T *begin = c_str() + pos;
    const T *found;
    if constexpr (std::is_same_v<T, char>)
        found = strstr(begin, substr);
    else
        found = wcsstr(begin, substr);
    if (!found)
        return npos;
    return found - begin + pos;
}

template<class T, size_t N>
template<size_t M>
inline size_t inplace_string<T, N>::find(const inplace_string<T, M>& substr, size_t pos /* 0 */) const noexcept
{
    return find(substr.c_str(), pos);
}

template<class T, size_t N>
inline size_t inplace_string<T, N>::copy(T *dst, size_t count, size_t pos /* 0 */) const noexcept
{
    size_t len = length();
    assert(pos <= len);
    const T *src = c_str() + pos;
    count = std::min(count, len - pos);
    for (size_t i = 0; i < count; ++i)
        dst[i] = src[i];
    return count;
}

template<class T, size_t N>
inline inplace_string<T, N> inplace_string<T, N>::substr(size_t pos, size_t count) const noexcept
{
    size_t len = length();
    if (pos >= len)
        return inplace_string();
    count = std::min(count, len - pos);
    if (literal() && (T('\0') == lit_str[pos + count]))
        return inplace_string(lit_str, pos, count);
    const T *first = begin() + pos;
    inplace_string sub;
    if (count <= N) [[likely]]
        sub.copy_inplace(first, count);
    else [[unlikely]]
        sub.spill(first, count);
    return sub;
}

template<class T, size_t N>
inline inplace_string<T, N>& inplace_string<T, N>::replace(T old, T new_) noexcept
{
    assert(old != new_);
    size_t pos = find(old);
    if (npos == pos)
        return *this;
    if (literal())
        copy_on_write();
    T *ch = begin() + pos;
    *ch++ = new_;
    while (*ch)
    {
        if (*ch == old)
            *ch = new_;
        ++ch;
    }
    if (spilled())
        uid = Unhashed;
    return *this;
}

template<class T, size_t N>
template<size_t M>
inline inplace_string<T, N>& inplace_string<T, N>::replace(size_t pos, size_t count, const inplace_string<T, M>& string) noexcept
{
    const size_t len = length();
    assert(pos <= len);
    if (pos > len)
        return *this;
    count = std::min(count, string.length());
    assert(pos + count <= len);
    if (pos + count > len)
        return *this;
    if (literal() && len <= N)
    {   // can fit in-situ
        copy_inplace(lit_str, len);
        return replace(pos, count, string);
    }
    else if (insitu()) [[likely]]
        memcpy(&buf[pos], string.c_str(), count * sizeof(T));
    else [[unlikely]]
    {
        if (!spilled())
            spill(c_str(), len);
        memcpy(str + pos, string.c_str(), count * sizeof(T));
    }
    if (spilled())
        uid = Unhashed;
    return *this;
}

template<class T, size_t N>
template<size_t M>
inline inplace_string<T, N>& inplace_string<T, N>::concat(const inplace_string<T, M>& string) noexcept
{
    if (literal())
    {
        copy_on_write();
        return concat(string);
    }
    size_t len1 = length();
    size_t len2 = string.length();
    if (insitu()) [[likely]]
    {
        if (len1 + len2 <= N) [[likely]]
        {
            string_concat(buf, string.c_str());
            buf[Capacity] -= (T)len2;
        }
        else [[unlikely]]
        {
            spill(buf, len1);
            return concat(string);
        }
    }
    else [[unlikely]]
    {
        if (len2 <= cap) [[likely]]
        {
            string_concat(str, string.c_str());
            cap -= len2;
        }
        else [[unlikely]]
        {
            len += len2;
            if (void *dst = realloc(str, bytes_size()))
            {
                str = string_concat((T *)dst, string.c_str());
                cap = 0;
            }
        }
        uid = Unhashed;
    }
    return *this;
}

template<class T, size_t N>
template<size_t M>
inline inplace_string<T, N>& inplace_string<T, N>::concat(const T (&str)[M]) noexcept
{
    return concat(inplace_string<T, std::max(N, M)>(str));
}

template<class T, size_t N>
inline inplace_string<char, N> inplace_string<T, N>::ansi() const noexcept
{
    if constexpr (std::is_same_v<T, wchar_t>)
    {
        inplace_string<char, N> dst;
        const wchar_t *src = c_str();
        size_t len = wcstombs(nullptr, src, 0);
        if (size_t(-1) == len) // conversion error
            return dst;
        if (len <= N)
            dst.buf[Capacity] = char(N - wcstombs(dst.buf, src, N + 1));
        else if (dst.str = (char *)malloc(len + 1))
        {
            len = wcstombs(dst.str, src, len + 1);
            dst.init(len, 0, dst.Spilled);
        }
        return dst;
    }
    else return *this;
}

template<class T, size_t N>
inline inplace_string<wchar_t, N> inplace_string<T, N>::wide() const noexcept
{
    if constexpr (std::is_same_v<T, char>)
    {
        inplace_string<wchar_t, N> dst;
        const char *src = c_str();
        size_t len = mbstowcs(nullptr, src, 0);
        if (size_t(-1) == len) // conversion error
            return dst;
        if (len <= N)
            dst.buf[Capacity] = wchar_t(N - mbstowcs(dst.buf, src, N + 1));
        else
        {
            size_t size = (len + 1) * sizeof(wchar_t);
            if (dst.str = (wchar_t *)malloc(size))
            {
                len = mbstowcs(dst.str, src, len + 1);
                dst.init(len, 0, dst.Spilled);
            }
        }
        return dst;
    }
    else return *this;
}

template<class T, size_t N>
template<class U>
inline U inplace_string<T, N>::to() const noexcept
{
    static_assert(std::is_integral_v<U> || std::is_floating_point_v<U>,
        "inplace_string::to(): only intergral and floating point types are supported");
    U number(0);
    T *end = nullptr;
    if constexpr (std::is_same_v<T, char>)
    {
        if constexpr (std::is_integral_v<U>)
        {
            if (std::is_signed_v<U>)
                number = (U)strtoll(c_str(), &end, 10);
            else if (std::is_unsigned_v<U>)
                number = (U)strtoull(c_str(), &end, 10);
        }
        else if constexpr (std::is_floating_point_v<U>)
        {
            if constexpr (std::is_same_v<U, float>)
                number = (U)strtof(c_str(), &end);
            else
                number = (U)strtod(c_str(), &end);
        }
    }
    else if constexpr(std::is_same_v<T, wchar_t>)
    {
        if constexpr (std::is_integral_v<U>)
        {
            if (std::is_signed_v<U>)
                number = (U)wcstoll(c_str(), &end, 10);
            else if (std::is_unsigned_v<U>)
                number = (U)wcstoull(c_str(), &end, 10);
        }
        else if constexpr (std::is_floating_point_v<U>)
        {
            if constexpr (std::is_same_v<U, float>)
                number = (U)wcstof(c_str(), &end);
            else
                number = (U)wcstod(c_str(), &end);
        }
    }
    assert(T('\0') == *end);
    return number;
}

template<class T, size_t N>
inline uint32_t inplace_string<T, N>::hash() const noexcept
{
    uint32_t hash;
    if (insitu()) [[likely]]
        hash = fnv1(buf, N - buf[Capacity]);
    else [[unlikely]]
    {
        hash = fnv1(lit_str, len);
        uid = hash;
    }
    return hash;
}

template<class T, size_t N>
inline bool inplace_string<T, N>::hashed() const noexcept
{
    return !insitu() && (uid != Unhashed);
}

template<class T, size_t N>
inline inplace_string<T, N>& inplace_string<T, N>::operator=(const inplace_string& string) noexcept
{
    if (string.literal())
    {
        back_to_insitu();
        str = string.str;
        init(string.len, 0, Literal, string.uid);
    }
    else if (string.insitu()) [[likely]]
    {
        back_to_insitu();
        copy_inplace(string.buf, string.length());
    }
    else [[unlikely]] /* spilled */
    {
        if (!spilled())
            spill(string.c_str(), string.length());
        else
            replace_spilled(string);
    }
    return *this;
}

template<class T, size_t N>
template<size_t M>
inline inplace_string<T, N>& inplace_string<T, N>::operator=(const inplace_string<T, M>& string) noexcept
{
    if (string.literal())
    {
        back_to_insitu();
        str = string.str;
        init(string.len, 0, Literal, string.uid);
    }
    else if (string.length() <= N)
    {
        back_to_insitu();
        copy_inplace(string.c_str(), string.length());
    }
    else
    {
        if (!spilled())
            spill(string.c_str(), string.length());
        else
            replace_spilled(string);
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
    if (lazy_hash() && s.lazy_hash())
    {
        if (uid != s.uid) [[likely]]
            return false;
    }
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
    init(length, 0, Literal);
}

template<class T, size_t N>
template<size_t M>
inline void inplace_string<T, N>::copy_ctor(const inplace_string<T, M>& s) noexcept
{
    size_t len = s.length();
    if (len <= N) [[likely]]
        copy_inplace(s.c_str(), len);
    else [[unlikely]]
        copy_heap(s.c_str(), len, s.bytes_size());
}

template<class T, size_t N>
inline void inplace_string<T, N>::copy_inplace(const T *c_str, size_t length) noexcept
{
    assert(!spilled()); // Don't overwrite heap pointer
    assert(length <= N);
    memcpy(buf, c_str, length * sizeof(T));
    buf[length] = T('\0');
    buf[Capacity] = T(N - length);
}

template<class T, size_t N>
inline void inplace_string<T, N>::copy_heap(const T *src, size_t length, size_t size) noexcept
{
    size_t count = (length + 1) * sizeof(T);
    assert(count <= size);
    void *dst = malloc(size);
    if (dst)
    {
        str = (T *)memcpy(dst, src, count); // including '\0'
        init(length, size - count, Spilled);
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
        uint32_t hash = hashed() ? (uint32_t)uid : Unhashed;
        init(length, space - length - 1, Spilled, hash);
    }
}

template<class T, size_t N>
inline void inplace_string<T, N>::back_to_insitu() noexcept
{
    if (spilled())
    {
        free(str);
        reset();
    }
}

template<class T, size_t N>
template<size_t M>
inline void inplace_string<T, N>::replace_spilled(const inplace_string<T, M>& string)
{
    size_t length = string.length();
    size_t size = string.bytes_size();
    T *dst;
    if (len + cap >= length)
    {
        dst = str;
        cap -= (length - len);
    }
    else
    {
        dst = (T *)realloc(str, size);
        if (dst) cap = 0;
    }
    if (dst)
    {
        str = (T *)memcpy(dst, string.c_str(), size);
        init(length, cap, Spilled);
    }
}

template<class T, size_t N>
inline void inplace_string<T, N>::copy_on_write() noexcept
{
    assert(literal());
    if (len <= N) [[likely]]
        copy_inplace(lit_str, len);
    else
        spill(lit_str, len);
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
    init(other.len, other.cap, other.spilled() ? Spilled : Literal, other.uid);
    other.reset();
}

template<class T, size_t N>
inline void inplace_string<T, N>::init(size_t length, size_t capacity, T flag, uint32_t hash /* Unhashed */) noexcept
{
    len = length;
    cap = capacity;
    uid = hash;
    buf[Capacity] = flag;
}

template<class T, size_t N>
inline void inplace_string<T, N>::reset() noexcept
{
    buf[0] = T('\0');
    buf[Capacity] = N;
}

template<class T, size_t N>
inline bool inplace_string<T, N>::lazy_hash() const noexcept
{
    return (!insitu()) && ((uid != Unhashed) || (hash(), true));
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
