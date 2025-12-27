#pragma once
#include <stdint.h>
#include "fnv_hash.h"

template<class T>
class literal_string
{
public:
    template<size_t N>
    constexpr literal_string(const T (&str)[N]):
        str(str), len(N - 1), uid(fnv1(str)) {}
    constexpr const T* c_str() const { return str; }
    constexpr size_t length() const { return len; }
    constexpr uint32_t hash() const { return uid; }

private:
    const T *const str;
    const size_t len;
    const uint32_t uid;
};

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)

#define Literal(str)\
([]() constexpr {\
    constexpr literal_string<char> lit(str);\
    return lit;\
}())

#define WLiteral(str)\
([]() constexpr {\
    constexpr literal_string<wchar_t> lit(WIDEN(str));\
    return lit;\
}())
