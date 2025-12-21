#include "inplace_string.h"

void print(const string<>& s)
{
    std::cout << "\"" << s
        << "\" : length: " << s.length() <<
        ", capacity: " << s.capacity() <<
        ", in-situ: " << std::boolalpha << s.insitu()
        << std::endl;
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
    appendCharsTest();
    std::cout << std::endl;

    const string<> he("Johnny"), she("Molly");
    doComparisons(he, she);
    std::cout << std::endl;
    doComparisonsWithCStr(she, "Molly");
    std::cout << std::endl;

    return 0;
}
