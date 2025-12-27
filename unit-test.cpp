#include "inplace_string.h"
#include <initializer_list>

template<class T, size_t N>
void print(const inplace_string<T, N>& s)
{
    if constexpr (std::is_same_v<T, char>)
        std::cout << "\"" << s;
    else
        std::wcout << L"\"" << s;
    std::cout << "\" : len=" << s.length()
        << ", cap=" << s.capacity()
        << ", in-situ: " << std::boolalpha << s.insitu()
        << ", literal: " << std::boolalpha << s.literal();
    if (s.hashed())
        std::cout << ", hash=" << s.hash();
    std::cout << std::endl;
}

void literalStringTest()
{
    const string<> str("Compile-time literal string");
    assert(str.length() > 0);
    assert(str.capacity() == 0);
    assert(!str.empty());
    assert(!str.insitu());
    assert(!str.spilled());
    assert(str.literal());
    for (auto const& it: str)
        std::cout << it << ',';
    std::cout << std::endl;
    print(str);
}

void moveContructorTest()
{
    string<25> a;
    string<16> b;
    string<20> c;
    a = "This string spilled to heap";
    b = "In-situ string";
    c = "In-situ string too";
    string<> triviallyMoved(std::move(a));
    string<> copiedInsitu(std::move(b));
    string<> insituSpilled(std::move(c));
    assert(a.empty());
    assert(b.empty());
    assert(c.empty());
    print(triviallyMoved);
    print(copiedInsitu);
    print(insituSpilled);
}

void pushPopTest(const std::initializer_list<char>& chars)
{
    string<> s;
    for (auto ch: chars)
    {
        s.push_back(ch);
        std::cout << s << std::endl;
    }
    while (!s.empty())
    {
        std::cout << s << std::endl;
        s.pop_back();
    }
    assert(s.length() == 0);
    assert(s.empty());
}

void appendCharsTest()
{
    string<> s;
    print(s);
    constexpr char array[] = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    int i = 0;
    while (s.capacity())
        s += array[i++];
    print(s);
    s += array[i]; // spill
    print(s);
    constexpr char array2[] = {'q','r','s','t','u','v','w','x', 'y','z'};
    for (char ch: array2)
        s += ch;
    print(s);
}

void substringTest()
{
    std::cout << "Retrievieng sub-strings from literal:" << std::endl;
    string<> s("abcdefg");
    std::cout << s.substr(0) << std::endl;
    std::cout << s.substr(4) << std::endl;
    std::cout << s.substr(10) << std::endl;
    std::cout << "Retrievieng sub-strings from in-situ:" << std::endl;
    s = "abcdefg";
    std::cout << s.substr(0) << std::endl;
    std::cout << s.substr(2, 3) << std::endl;
    std::cout << s.substr(15) << std::endl;
    std::cout << "Retrievieng sub-strings from spilled:" << std::endl;
    s = "abcdefghijklmnopq";
    std::cout << s.substr(0) << std::endl;
    std::cout << s.substr(2, 3) << std::endl;
    std::cout << s.substr(15) << std::endl;
}

void doComparisons(const string<>& s1, const string<>& s2)
{
    std::cout << "Comparing two strings: \"" << s1 << "\" and \"" << s2 << "\"\n";
    std::cout << std::boolalpha
        << "operator < : " << (s1 < s2) << std::endl
        << "operator <= : " << (s1 <= s2) << std::endl
        << "operator > : " << (s1 > s2) << std::endl
        << "operator >= : " << (s1 >= s2) << std::endl
        << "operator == : " << (s1 == s2) << std::endl
        << "operator != : " << (s1 != s2) << std::endl;
}

void doComparisonsWithCStr(const string<>& s1, const char *s2)
{
    std::cout << "Comparing two strings: \"" << s1 << "\" and \"" << s2 << "\"\n";
    std::cout << std::boolalpha
        << "operator < : " << (s1 < s2) << std::endl
        << "operator <= : " << (s1 <= s2) << std::endl
        << "operator > : " << (s1 > s2) << std::endl
        << "operator >= : " << (s1 >= s2) << std::endl
        << "operator == : " << (s1 == s2) << std::endl
        << "operator != : " << (s1 != s2) << std::endl;
}

int main()
{
    pushPopTest({'a','b','c','d','e','f','g'});
    std::cout << std::endl;
    pushPopTest({'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q'});
    std::cout << std::endl;
    literalStringTest();
    std::cout << std::endl;
    moveContructorTest();
    std::cout << std::endl;
    appendCharsTest();
    std::cout << std::endl;
    substringTest();
    std::cout << std::endl;

    const string<> he("Johnny"), she("Molly");
    doComparisons(he, she);
    std::cout << std::endl;
    doComparisonsWithCStr(she, "Molly");
    std::cout << std::endl;

    return 0;
}
