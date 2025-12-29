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

void copyConstructorTest()
{
    const string<> readOnly(Literal("Read-only string"));
    string<> small, lengthy;
    small = "Small string";
    lengthy = "A bit lengthy string";
    string<> copiedReadOnly(readOnly);
    string<> copiedInsitu(small);
    string<> copiedSpilled(lengthy);
    string<20> copiedAsInsitu(lengthy);
    print(copiedReadOnly);
    print(copiedInsitu);
    print(copiedSpilled);
    print(copiedAsInsitu);
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


void characterReplaceTest()
{
    std::cout << "Replacing windows-style file path to unix-style:" << std::endl;
    string<> path(Literal("C:\\Users\\defunct\\file.txt"));
    std::cout << path << std::endl;
    path.replace('\\', '/');
    std::cout << path << std::endl;
}

void substringReplaceTest()
{
    std::cout << "Replacing sub-string:" << std::endl;
    // literal -> in-situ -> replace
    const string<> ket("ket");
    string<> basket("basics");
    std::cout << basket << " -> ";
    basket.replace(3, ket.length(), ket);
    std::cout << basket << std::endl;
    // init in-situ -> replace
    const string<> bat("bat");
    string<> battle;
    battle = "castle";
    std::cout << battle << " -> ";
    battle.replace(0, bat.length(), bat);
    std::cout << battle << std::endl;
    // literal -> spill -> replace
    string<> str("abcdefghijklmnop");
    size_t pos = str.find("ijk");
    assert(pos != string<>::npos);
    const string<> xyz("xyz");
    std::cout << str << " -> ";
    str.replace(pos, 3, xyz);
    std::cout << str << std::endl;
}

template<class String>
void toNumberConversionTest()
{
    std::cout << "Convert string to number:" << std::endl;
    const String noString;
    int zero = noString.to<int>();
    assert(0 == zero);
    if constexpr (std::is_same_v<typename String::type, wchar_t>)
    {
        const String empty(L"");
        double zero = empty.to<double>();
        assert(0 == zero);
    }
    else
    {
        const String empty("");
        double zero = empty.to<double>();
        assert(0 == zero);
    }
    String integral;
    String floatingPoint;
    if constexpr (std::is_same_v<typename String::type, wchar_t>)
    {
        integral = L"904259";
        floatingPoint = L"-12.956";
    }
    else
    {
        integral = "904259";
        floatingPoint = "-12.956";
    }
    int i = integral.to<int>();
    double d = floatingPoint.to<double>();
    std::cout << i << std::endl;
    std::cout << d << std::endl;
    // these should fail
    //string<>("4-23a45").to<int>();
    //string<>("[34.*743").to<float>();
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
    copyConstructorTest();
    std::cout << std::endl;
    moveContructorTest();
    std::cout << std::endl;
    appendCharsTest();
    std::cout << std::endl;
    substringTest();
    std::cout << std::endl;
    characterReplaceTest();
    std::cout << std::endl;
    substringReplaceTest();
    std::cout << std::endl;
    toNumberConversionTest<string<>>();
    toNumberConversionTest<wstring<>>();

    const string<> he("Johnny"), she("Molly");
    doComparisons(he, she);
    std::cout << std::endl;
    doComparisonsWithCStr(she, "Molly");
    std::cout << std::endl;

    return 0;
}
