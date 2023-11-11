#include <core_as/str/sstring.h>
#include <gtest/gtest.h>

COREAS_API void* core_as_malloc(size_t count) {
    return malloc(count);
}

COREAS_API void* core_as_realloc(void* ptr, size_t count) {
    return realloc(ptr, count);
}

COREAS_API void core_as_free(void* ptr) {
    free(ptr);
}

namespace core_as::str::tests {

class Tstringa: public stringa {
public:
    using stringa::stringa;

    bool isLocalString() const { return type == Local; }
    bool isConstantString() const { return type == Constant; }
    bool isSharedString() const { return type == Shared; }

    size_t sharedCount() const {
        return type == Shared ? SharedStringData<u8s>::from_str(sstr)->ref.load() : 0u;
    }
};


TEST(CoreAsStr, CreateSimpleEmpty) {
    ssa testa{nullptr, 0};
    EXPECT_TRUE(testa.isEmpty());
    EXPECT_EQ(testa.length(), 0u);

    ssw testw{nullptr, 0};
    EXPECT_TRUE(testw.isEmpty());
    EXPECT_EQ(testw.length(), 0u);

    ssu testu{nullptr, 0};
    EXPECT_TRUE(testu.isEmpty());
    EXPECT_EQ(testu.length(), 0u);

    ssuu testuu{nullptr, 0};
    EXPECT_TRUE(testuu.isEmpty());
    EXPECT_EQ(testuu.length(), 0u);
}

TEST(CoreAsStr, CompareSimpleEmptyEqual) {

    ssa testa{nullptr, 0};
    EXPECT_TRUE(testa == "");
    EXPECT_TRUE(testa != "test");

    ssw testw{nullptr, 0};
    EXPECT_TRUE(testw == L"");
    EXPECT_TRUE(testw != L"test");

    ssu testu{nullptr, 0};
    EXPECT_TRUE(testu == u"");
    EXPECT_TRUE(testu != u"test");

    ssuu testuu{nullptr, 0};
    EXPECT_TRUE(testuu == U"");
    EXPECT_TRUE(testuu != U"test");
}

TEST(CoreAsStr, CompareSimpleNonEmptyEqual) {

    ssa testa{"test"};
    EXPECT_TRUE(testa == "test");
    EXPECT_TRUE(testa != "");

    ssw testw{L"test"};
    EXPECT_TRUE(testw == L"test");
    EXPECT_TRUE(testw != L"");

    ssu testu{u"test"};
    EXPECT_TRUE(testu == u"test");
    EXPECT_TRUE(testu != u"");

    ssuu testuu{U"test"};
    EXPECT_TRUE(testuu == U"test");
    EXPECT_TRUE(testuu != U"");
}

TEST(CoreAsStr, CreateSimple) {

    ssa testa{"test"};
    EXPECT_EQ(testa, "test");
    EXPECT_FALSE(testa.isEmpty());

    ssu testu{u"test"};
    EXPECT_EQ(testu, u"test");
    EXPECT_FALSE(testu.isEmpty());

    ssw testw{L"test"};
    EXPECT_EQ(testw, L"test");
    EXPECT_FALSE(testw.isEmpty());
}

TEST(CoreAsStr, At) {
    ssa testa{"test"};
    EXPECT_EQ(testa.at(1), 'e');
    EXPECT_EQ(testa.at(-1), 't');
    EXPECT_EQ(testa.at(2), 's');
    EXPECT_EQ(testa.at(-2), 's');

    ssu testu{u"test"};
    EXPECT_EQ(testu.at(1), u'e');
    EXPECT_EQ(testu.at(-1), u't');
    EXPECT_EQ(testu.at(2), u's');
    EXPECT_EQ(testu.at(-2), u's');

    ssw testw{L"test"};
    EXPECT_EQ(testw.at(1), L'e');
    EXPECT_EQ(testw.at(-1), L't');
    EXPECT_EQ(testw.at(2), L's');
    EXPECT_EQ(testw.at(-2), L's');
}

TEST(CoreAsStr, CompareSimpleAll) {

    ssa testa1{"test"}, testa2{"testa"};
    EXPECT_TRUE(testa1 < "testa");
    EXPECT_TRUE(testa1 < testa2);
    EXPECT_TRUE(testa2 > testa1);
    EXPECT_TRUE(testa1 > "");
    EXPECT_TRUE(testa1 > "te");
    EXPECT_TRUE("" < testa1);
    EXPECT_TRUE("te" < testa1);
    EXPECT_TRUE("test" == testa1);
    EXPECT_TRUE(testa1 != "");
    EXPECT_TRUE(testa1 != testa2);
    EXPECT_EQ(testa1.compare(testa2), -1);
    EXPECT_EQ(testa2.compare(testa1), 1);
    EXPECT_EQ(testa1.compare(testa1), 0);
    testa2.len = testa1.length();
    EXPECT_TRUE(testa1 == testa2);


    ssu testu1{u"test"}, testu2{u"testa"};
    EXPECT_TRUE(testu1 < u"testa");
    EXPECT_TRUE(testu1 < testu2);
    EXPECT_TRUE(testu2 > testu1);
    EXPECT_TRUE(testu1 > u"");
    EXPECT_TRUE(testu1 > u"te");
    EXPECT_TRUE(u"" < testu1);
    EXPECT_TRUE(u"te" < testu1);
    EXPECT_TRUE(u"test" == testu1);
    EXPECT_TRUE(testu1 != u"");
    EXPECT_TRUE(testu1 != testu2);
    EXPECT_EQ(testu1.compare(testu2), -1);
    EXPECT_EQ(testu2.compare(testu1), 1);
    EXPECT_EQ(testu1.compare(testu1), 0);
    testu2.len = testu1.length();
    EXPECT_TRUE(testu1 == testu2);

    ssw testw1{L"test"}, testw2{L"testa"};
    EXPECT_TRUE(testw1 < L"testa");
    EXPECT_TRUE(testw1 < testw2);
    EXPECT_TRUE(testw2 > testw1);
    EXPECT_TRUE(testw1 > L"");
    EXPECT_TRUE(testw1 > L"te");
    EXPECT_TRUE(L"" < testw1);
    EXPECT_TRUE(L"te" < testw1);
    EXPECT_TRUE(L"test" == testw1);
    EXPECT_TRUE(testw1 != L"");
    EXPECT_TRUE(testw1 != testw2);
    EXPECT_EQ(testw1.compare(testw2), -1);
    EXPECT_EQ(testw2.compare(testw1), 1);
    EXPECT_EQ(testw1.compare(testw1), 0);
    testw2.len = testw1.length();
    EXPECT_TRUE(testw1 == testw2);
}

TEST(CoreAsStr, CompareSimpleAllIA) {

    ssa testa1{"tEst"}, testa2{"Testa"};
    EXPECT_TRUE(testa1.isEqual_ia("test"));
    EXPECT_TRUE(testa1 > testa2);
    EXPECT_TRUE(testa1.compare_ia(testa2) < 0);

    ssu testu1{u"tEst"}, testu2{u"Testa"};
    EXPECT_TRUE(testu1.isEqual_ia(u"test"));
    EXPECT_TRUE(testu1 > testu2);
    EXPECT_TRUE(testu1.compare_ia(testu2) < 0);

    ssw testw1{L"tEst"}, testw2{L"Testa"};
    EXPECT_TRUE(testw1.isEqual_ia(L"test"));
    EXPECT_TRUE(testw1 > testw2);
    EXPECT_TRUE(testw1.compare_ia(testw2) < 0);

    ssuu testuu1{U"tEst"}, testuu2{U"Testa"};
    EXPECT_TRUE(testuu1.isEqual_ia(U"test"));
    EXPECT_TRUE(testuu1 > testuu2);
    EXPECT_TRUE(testuu1.compare_ia(testuu2) < 0);
}

TEST(CoreAsStr, CompareSimpleAllIU) {

    ssa testa1{"шоРох"}, testa2{"ШороХа"};
    EXPECT_TRUE(testa1.isEqual_iu("Шорох"));
    EXPECT_TRUE(testa1 > testa2);
    EXPECT_TRUE(testa1.compare_iu(testa2) < 0);

    ssu testu1{u"шоРох"}, testu2{u"ШороХа"};
    EXPECT_TRUE(testu1.isEqual_iu(u"Шорох"));
    EXPECT_TRUE(testu1 > testu2);
    EXPECT_TRUE(testu1.compare_iu(testu2) < 0);

    ssw testw1{L"шоРох"}, testw2{L"ШороХа"};
    EXPECT_TRUE(testw1.isEqual_iu(L"Шорох"));
    EXPECT_TRUE(testw1 > testw2);
    EXPECT_TRUE(testw1.compare_iu(testw2) < 0);

    ssuu testuu1{U"шоРох"}, testuu2{U"ШороХа"};
    EXPECT_TRUE(testuu1.isEqual_iu(U"Шорох"));
    EXPECT_TRUE(testuu1 > testuu2);
    EXPECT_TRUE(testuu1.compare_iu(testuu2) < 0);
}

TEST(CoreAsStr, SimpleFind) {
    ssa testa = "Find a needle in a haystack";
    EXPECT_EQ(testa.find("diamond"), str_pos::badIdx);
    EXPECT_EQ(testa.find("needle"), 7u);
    EXPECT_EQ(testa.find("a"), 5u);
    EXPECT_EQ(testa.find("a", 8), 17u);
    EXPECT_EQ(testa.find('a'), 5u);
    EXPECT_EQ(testa.find('a', 8), 17u);
    EXPECT_EQ(testa.findReverse('d'), 10u);

    auto res = testa.findAll("i");
    EXPECT_EQ(res.size(), 2u);
    EXPECT_EQ(res[0], 1u);
    EXPECT_EQ(res[1], 14u);

    res = testa.findAll("i", 8);
    EXPECT_EQ(res.size(), 1u);
    EXPECT_EQ(res[0], 14u);

    res = testa.findAll("i", 1, 1);
    EXPECT_EQ(res.size(), 1u);
    EXPECT_EQ(res[0], 1u);

    ssu testu = u"Find a needle in a haystack";
    EXPECT_EQ(testu.find(u"diamond"), str_pos::badIdx);
    EXPECT_EQ(testu.find(u"needle"), 7u);
    EXPECT_EQ(testu.find(u"a"), 5u);
    EXPECT_EQ(testu.find(u"a", 8), 17u);
    EXPECT_EQ(testu.find(u'a'), 5u);
    EXPECT_EQ(testu.find(u'a', 8), 17u);
    EXPECT_EQ(testu.findReverse(u'd'), 10u);

    res = testu.findAll(u"i");
    EXPECT_EQ(res.size(), 2u);
    EXPECT_EQ(res[0], 1u);
    EXPECT_EQ(res[1], 14u);

    ssw testw = L"Find a needle in a haystack";
    EXPECT_EQ(testw.find(L"diamond"), str_pos::badIdx);
    EXPECT_EQ(testw.find(L"needle"), 7u);
    EXPECT_EQ(testw.find(L"a"), 5u);
    EXPECT_EQ(testw.find(L"a", 8), 17u);
    EXPECT_EQ(testw.find(L'a'), 5u);
    EXPECT_EQ(testw.find(L'a', 8), 17u);
    EXPECT_EQ(testw.findReverse(L'd'), 10u);

    res = testw.findAll(L"i");
    EXPECT_EQ(res.size(), 2u);
    EXPECT_EQ(res[0], 1u);
    EXPECT_EQ(res[1], 14u);
}

TEST(CoreAsStr, SubPiece) {

    ssa testa{"test"};
    EXPECT_EQ(testa(1), "est");
    EXPECT_EQ(testa(1, 2), "es");

    ssu testu{u"test"};
    EXPECT_EQ(testu(1), u"est");
    EXPECT_EQ(testu(1, 2), u"es");

    ssw testw{L"test"};
    EXPECT_EQ(testw(1), L"est");
    EXPECT_EQ(testw(1, 2), L"es");
}

TEST(CoreAsStr, SimpleSubstr) {
    ssa testa = "test";
    EXPECT_EQ(testa.substr(1, 0), "est");
    EXPECT_EQ(testa.substr(1, 2), "es");
    EXPECT_EQ(testa.substr(0, -1), "tes");
    EXPECT_EQ(testa.substr(-2), "st");
    EXPECT_EQ(testa.substr(-3, 2), "es");
    EXPECT_EQ(testa.substr(-3, -1), "es");
    EXPECT_EQ(testa.strMid(1), "est");
    EXPECT_EQ(testa.strMid(1, 0), "");
    EXPECT_EQ(testa.strMid(1, 2), "es");
    EXPECT_EQ(testa.strMid(2, 2), "st");

    ssu testu = u"test";
    EXPECT_EQ(testu.substr(1, 0), u"est");
    EXPECT_EQ(testu.substr(1, 2), u"es");
    EXPECT_EQ(testu.substr(0, -1), u"tes");
    EXPECT_EQ(testu.substr(-2), u"st");
    EXPECT_EQ(testu.substr(-3, 2), u"es");
    EXPECT_EQ(testu.substr(-3, -1), u"es");
    EXPECT_EQ(testu.strMid(1), u"est");
    EXPECT_EQ(testu.strMid(1, 0), u"");
    EXPECT_EQ(testu.strMid(1, 2), u"es");
    EXPECT_EQ(testu.strMid(2, 2), u"st");

    ssw testw = L"test";
    EXPECT_EQ(testw.substr(1, 0), L"est");
    EXPECT_EQ(testw.substr(1, 2), L"es");
    EXPECT_EQ(testw.substr(0, -1), L"tes");
    EXPECT_EQ(testw.substr(-2), L"st");
    EXPECT_EQ(testw.substr(-3, 2), L"es");
    EXPECT_EQ(testw.substr(-3, -1), L"es");
    EXPECT_EQ(testw.strMid(1), L"est");
    EXPECT_EQ(testw.strMid(1, 0), L"");
    EXPECT_EQ(testw.strMid(1, 2), L"es");
    EXPECT_EQ(testw.strMid(2, 2), L"st");
}

TEST(CoreAsStr, ToInt) {
    EXPECT_EQ(ssa{"  123"}.asInt<int>(), 123);
    EXPECT_EQ(ssa{"  123"}(0, -1).asInt<int>(), 12);
    EXPECT_EQ(ssa{"+123"}.asInt<int>(), 123);
    EXPECT_EQ(ssa{"  -123aa"}.asInt<int>(), -123);
    EXPECT_EQ(ssa{"123"}.asInt<size_t>(), 123u);
    EXPECT_EQ(ssa{"-123"}.asInt<size_t>(), 0u);
    EXPECT_EQ(ssa{"   0123  "}.asInt<int>(), 0123);
    EXPECT_EQ(ssa{"-0x123"}.asInt<int>(), -0x123);

    {
        auto [number, err, len] = ssa{"asd"}.toInt<int>();
        EXPECT_EQ(number, 0);
        EXPECT_EQ(err, IntConvertResult::NotNumber);
        EXPECT_EQ(len, 0u);
    }

    {
        auto [number, err, len] = ssa{"0"}.toInt<int>();
        EXPECT_EQ(number, 0);
        EXPECT_EQ(err, IntConvertResult::Success);
        EXPECT_EQ(len, 1u);
    }

    {
        auto [number, err, len] = ssa{"1"}.toInt<int>();
        EXPECT_EQ(number, 1);
        EXPECT_EQ(err, IntConvertResult::Success);
        EXPECT_EQ(len, 1u);
    }

    {
        auto [number, err, len] = ssa{"-"}.toInt<int>();
        EXPECT_EQ(number, 0);
        EXPECT_EQ(err, IntConvertResult::NotNumber);
        EXPECT_EQ(len, 1u);
    }

    {
        auto [number, err, len] = ssa{"+"}.toInt<int>();
        EXPECT_EQ(number, 0);
        EXPECT_EQ(err, IntConvertResult::NotNumber);
        EXPECT_EQ(len, 1u);
    }

    {
        auto [number, err, len] = ssa{"-0"}.toInt<int>();
        EXPECT_EQ(number, 0);
        EXPECT_EQ(err, IntConvertResult::Success);
        EXPECT_EQ(len, 2u);
    }

    {
        auto [number, err, len] = ssa{"+0"}.toInt<int>();
        EXPECT_EQ(number, 0);
        EXPECT_EQ(err, IntConvertResult::Success);
        EXPECT_EQ(len, 2u);
    }

    {
        auto [number, err, len] = ssa{"-0sd"}.toInt<int>();
        EXPECT_EQ(number, 0);
        EXPECT_EQ(err, IntConvertResult::BadSymbolAtTail);
        EXPECT_EQ(len, 2u);
    }
    {
        auto [number, err, len] = ssa{"1234"}.toInt<int8_t>();
        EXPECT_EQ(number, 123);
        EXPECT_EQ(err, IntConvertResult::Overflow);
        EXPECT_EQ(len, 3u);
    }
    {
        auto [number, err, len] = ssa{"128"}.toInt<int8_t>();
        EXPECT_EQ(number, 12);
        EXPECT_EQ(err, IntConvertResult::Overflow);
        EXPECT_EQ(len, 2u);
    }
    {
        auto [number, err, len] = ssa{"127"}.toInt<int8_t>();
        EXPECT_EQ(number, 127);
        EXPECT_EQ(err, IntConvertResult::Success);
        EXPECT_EQ(len, 3u);
    }
    {
        auto [number, err, len] = ssa{"-128"}.toInt<int8_t>();
        EXPECT_EQ(number, -128);
        EXPECT_EQ(err, IntConvertResult::Success);
        EXPECT_EQ(len, 4u);
    }

    {
        auto [number, err, len] = ssa{"0xFFFfffFF"}.toInt<size_t>();
        EXPECT_EQ(number, 0xFFFFFFFF);
        EXPECT_EQ(err, IntConvertResult::Success);
        EXPECT_EQ(len, 10u);
    }

    {
        auto [number, err, len] = ssa{"0x"}.toInt<size_t>();
        EXPECT_EQ(number, 0u);
        EXPECT_EQ(err, IntConvertResult::NotNumber);
        EXPECT_EQ(len, 2u);
    }

    {
        auto [number, err, len] = ssa{"0xS"}.toInt<size_t>();
        EXPECT_EQ(number, 0u);
        EXPECT_EQ(err, IntConvertResult::NotNumber);
        EXPECT_EQ(len, 2u);
    }

    {
        auto [number, err, len] = ssa{"-0x80000000"}.toInt<int>();
        EXPECT_EQ(number, -2147483648);
        EXPECT_EQ(err, IntConvertResult::Success);
        EXPECT_EQ(len, 11u);
    }

    {
        auto [number, err, len] = ssa{"-0x80000001"}.toInt<int>();
        EXPECT_EQ(number, -134217728);
        EXPECT_EQ(err, IntConvertResult::Overflow);
        EXPECT_EQ(len, 10u);
    }

    {
        auto [number, err, len] = ssa{"-0x80000001"}.toInt<int64_t>();
        EXPECT_EQ(number, -2147483649);
        EXPECT_EQ(err, IntConvertResult::Success);
        EXPECT_EQ(len, 11u);
    }

    EXPECT_EQ(ssu{u"  123"}.asInt<int>(), 123);
    EXPECT_EQ(ssu{u"+123"}.asInt<int>(), 123);
    EXPECT_EQ(ssu{u"  -123aa"}.asInt<int>(), -123);
    EXPECT_EQ(ssu{u"123"}.asInt<size_t>(), 123u);
    EXPECT_EQ(ssu{u"-123"}.asInt<size_t>(), 0u);
    EXPECT_EQ(ssu{u"   0123  "}.asInt<int>(), 0123);
    EXPECT_EQ(ssu{u"-0x123"}.asInt<int>(), -0x123);

    EXPECT_EQ(ssw{L"  123"}.asInt<int>(), 123);
    EXPECT_EQ(ssw{L"+123"}.asInt<int>(), 123);
    EXPECT_EQ(ssw{L"  -123aa"}.asInt<int>(), -123);
    EXPECT_EQ(ssw{L"123"}.asInt<size_t>(), 123u);
    EXPECT_EQ(ssw{L"-123"}.asInt<size_t>(), 0u);
    EXPECT_EQ(ssw{L"   0123  "}.asInt<int>(), 0123);
    EXPECT_EQ(ssw{L"-0x123"}.asInt<int>(), -0x123);

    int num1;
    uint64_t num2;
    double num3;
    ssa{"-10"}.asNumber(num1);
    EXPECT_EQ(num1, -10);
    ssa{"0xDeadBeef"}.asNumber(num2);
    EXPECT_EQ(num2, 0xdeadbeef);
    ssa{"0.128"}.asNumber(num3);
    EXPECT_EQ(num3, 0.128);
}

TEST(CoreAsStr, ToDouble) {
    EXPECT_EQ(ssa{"  123"}.toDouble(), 123.0);
    EXPECT_EQ(ssa{"  123.1"}.toDouble(), 123.1);
}

TEST(CoreAsStr, Split) {
    auto res = ssa{"1,2,3"}.split<std::vector<ssa>>(",");
    EXPECT_EQ(res.size(), 3u);
    EXPECT_EQ(res[0], "1");
    EXPECT_EQ(res[1], "2");
    EXPECT_EQ(res[2], "3");
}

TEST(CoreAsStr, Parts) {
    ssa test = "prefixВСтрокеSuffix";

    EXPECT_TRUE(test.starts_with("prefix"));
    EXPECT_TRUE(test.ends_with("Suffix"));
    EXPECT_TRUE(ssa{"prefix"}.isPrefixIn(test));
    EXPECT_FALSE(ssa{"prefix"}.starts_with("prefixa"));

    EXPECT_FALSE(test.starts_with("predix"));
    EXPECT_FALSE(test.ends_with("suffex"));
    EXPECT_FALSE(ssa{"predix"}.isPrefixIn(test));

    EXPECT_TRUE(test.starts_with_ia("Prefix"));
    EXPECT_TRUE(test.ends_with_ia("suffix"));

    EXPECT_TRUE(test.starts_with_iu("Prefixвс"));
    EXPECT_TRUE(test.ends_with_iu("КЕsuffix"));
}

TEST(CoreAsStr, IsAscii) {
    ssa test = "prefixВСтрокеSuffix";

    EXPECT_FALSE(test.isAscii());
    EXPECT_TRUE(ssa{"1234567812345678124"}.isAscii());
    EXPECT_TRUE(ssa{"asldkafjk^%&*&623216187263&^hjgaiu&&!^@*^?>"}.isAscii());
}

TEST(CoreAsStr, ChangeCase) {
    EXPECT_EQ(ssa{"TEST"}.upperedOnlyAscii<stringa>(), "TEST");
    EXPECT_EQ(ssa{"Test"}.upperedOnlyAscii<stringa>(), "TEST");
    EXPECT_EQ(ssa{"test"}.loweredOnlyAscii<stringa>(), "test");
    EXPECT_EQ(ssa{"Test"}.loweredOnlyAscii<stringa>(), "test");
    EXPECT_EQ(ssa{"TestПрОвЕрКа"}.upperedOnlyAscii<stringa>(), "TESTПрОвЕрКа");
    EXPECT_EQ(ssa{"TestПрОвЕрКа"}.loweredOnlyAscii<stringa>(), "testПрОвЕрКа");
    EXPECT_EQ(ssa{"TestПрОвЕрКа"}.uppered<stringa>(), "TESTПРОВЕРКА");
    EXPECT_EQ(ssa{"tesTПрОвЕрКа"}.lowered<stringa>(), "testпроверка");
    EXPECT_EQ(ssa{"TEST"}.uppered<stringa>(), "TEST");
    EXPECT_EQ(ssa{"test"}.lowered<stringa>(), "test");
    EXPECT_EQ(ssa{"TESTПРОВЕРКА"}.uppered<stringa>(), "TESTПРОВЕРКА");
    EXPECT_EQ(ssa{"testпроверка"}.lowered<stringa>(), "testпроверка");
    EXPECT_EQ(ssa{"tesTİiȺⱥȾⱦẞßΩФывAsd"}.lowered<stringa>(), "testiiⱥⱥⱦⱦßßωфывasd");
    EXPECT_EQ(ssa{"testⱢɱⱮɽⱤWas"}.uppered<stringa>(), "TESTⱢⱮⱮⱤⱤWAS");

    EXPECT_EQ(ssu{u"TEST"}.upperedOnlyAscii<stringu>(), u"TEST");
    EXPECT_EQ(ssu{u"test"}.loweredOnlyAscii<stringu>(), u"test");
    EXPECT_EQ(ssu{u"Test"}.upperedOnlyAscii<stringu>(), u"TEST");
    EXPECT_EQ(ssu{u"Test"}.loweredOnlyAscii<stringu>(), u"test");
    EXPECT_EQ(ssu{u"TestПрОвЕрКа"}.upperedOnlyAscii<stringu>(), u"TESTПрОвЕрКа");
    EXPECT_EQ(ssu{u"TestПрОвЕрКа"}.loweredOnlyAscii<stringu>(), u"testПрОвЕрКа");
    EXPECT_EQ(ssu{u"TestПрОвЕрКа"}.uppered<stringu>(), u"TESTПРОВЕРКА");
    EXPECT_EQ(ssu{u"tesTПрОвЕрКа"}.lowered<stringu>(), u"testпроверка");
    EXPECT_EQ(ssu{u"tesTİiȺⱥȾⱦẞßΩФывAsd"}.lowered<stringu>(), u"testiiⱥⱥⱦⱦßßωфывasd");
    EXPECT_EQ(ssu{u"testⱢɱⱮɽⱤWas"}.uppered<stringu>(), u"TESTⱢⱮⱮⱤⱤWAS");

    EXPECT_EQ(ssw{L"TEST"}.upperedOnlyAscii<stringw>(), L"TEST");
    EXPECT_EQ(ssw{L"test"}.loweredOnlyAscii<stringw>(), L"test");
    EXPECT_EQ(ssw{L"Test"}.upperedOnlyAscii<stringw>(), L"TEST");
    EXPECT_EQ(ssw{L"Test"}.loweredOnlyAscii<stringw>(), L"test");
    EXPECT_EQ(ssw{L"TestПрОвЕрКа"}.upperedOnlyAscii<stringw>(), L"TESTПрОвЕрКа");
    EXPECT_EQ(ssw{L"TestПрОвЕрКа"}.loweredOnlyAscii<stringw>(), L"testПрОвЕрКа");
    EXPECT_EQ(ssw{L"TestПрОвЕрКа"}.uppered<stringw>(), L"TESTПРОВЕРКА");
    EXPECT_EQ(ssw{L"tesTПрОвЕрКа"}.lowered<stringw>(), L"testпроверка");
    EXPECT_EQ(ssw{L"tesTİiȺⱥȾⱦẞßΩФывAsd"}.lowered<stringw>(), L"testiiⱥⱥⱦⱦßßωфывasd");
    EXPECT_EQ(ssw{L"testⱢɱⱮɽⱤWas"}.uppered<stringw>(), L"TESTⱢⱮⱮⱤⱤWAS");
}

TEST(CoreAsStr, Replace) {
    EXPECT_EQ(ssa{"testing"}.replaced<stringa>("t", "--"), "--es--ing");
    EXPECT_EQ(ssa{"testing"}.replaced<stringa>("t", ""), "esing");
    EXPECT_EQ(stringa{ssa{"testing"}.replace_init("t", "--")}, "--es--ing");
}

TEST(CoreAsStr, Trim) {
    EXPECT_EQ(ssa{"testing"}.trimmed(), "testing");
    EXPECT_EQ(ssa{"  \ttesting  "}.trimmed(), "testing");
    EXPECT_EQ(ssa{"  \ttesting  "}.trimmedLeft(), "testing  ");
    EXPECT_EQ(ssa{"  \ttesting  "}.trimmedRight(), "  \ttesting");
    EXPECT_EQ(ssa{"testing"}.trimmed("tg"), "estin");
    EXPECT_EQ(ssa{"testing"}.trimmedLeft("tg"), "esting");
    EXPECT_EQ(ssa{"testing"}.trimmedRight("tg"), "testin");
    EXPECT_EQ(ssa{"  \ttesting  "}.trimmedWithSpaces("tg"), "estin");
    EXPECT_EQ(ssa{"  \ttesting  "}.trimmedLeftWithSpaces("tg"), "esting  ");
    EXPECT_EQ(ssa{"  \ttesting  "}.trimmedRightWithSpaces("tg"), "  \ttestin");
    EXPECT_EQ(ssa{"testing"}.trimmed(ssa{"tg"}), "estin");
    EXPECT_EQ(ssa{"testing"}.trimmedLeft(ssa{"tg"}), "esting");
    EXPECT_EQ(ssa{"testing"}.trimmedRight(ssa{"tg"}), "testin");
    EXPECT_EQ(ssa{"  \ttesting  "}.trimmedWithSpaces(ssa{"tg"}), "estin");
    EXPECT_EQ(ssa{"  \ttesting  "}.trimmedLeftWithSpaces(ssa{"tg"}), "esting  ");
    EXPECT_EQ(ssa{"  \ttesting  "}.trimmedRightWithSpaces(ssa{"tg"}), "  \ttestin");
}

TEST(CoreAsStr, CreateSstringEmpty) {

    stringa testa;
    EXPECT_EQ(testa.length(), 0u);
    EXPECT_EQ(testa, "");
    EXPECT_TRUE(testa.isEmpty());

    stringu testu;
    EXPECT_EQ(testu.length(), 0u);
    EXPECT_EQ(testu, u"");
    EXPECT_TRUE(testu.isEmpty());

    stringw testw;
    EXPECT_EQ(testw.length(), 0u);
    EXPECT_EQ(testw, L"");
    EXPECT_TRUE(testw.isEmpty());
}

TEST(CoreAsStr, CreateSstringFromLiteral) {

    stringa testa{"test"};
    EXPECT_EQ(testa, "test");
    EXPECT_FALSE(testa.isEmpty());

    stringu testu{u"test"};
    EXPECT_EQ(testu, u"test");
    EXPECT_FALSE(testu.isEmpty());

    stringw testw{L"test"};
    EXPECT_EQ(testw, L"test");
    EXPECT_FALSE(testw.isEmpty());
}

TEST(CoreAsStr, CreateSstringCopy) {

    stringa testa{"test"};
    stringa copya{testa};
    EXPECT_EQ(copya, testa);
    EXPECT_EQ(copya, "test");
    EXPECT_FALSE(testa.isEmpty());
    EXPECT_FALSE(copya.isEmpty());

    stringu testu{u"test"};
    stringu copyw{testu};
    EXPECT_EQ(copyw, testu);
    EXPECT_EQ(copyw, u"test");
    EXPECT_FALSE(testu.isEmpty());
    EXPECT_FALSE(copyw.isEmpty());

    stringw testw{L"test"};
    stringw copyu{testw};
    EXPECT_EQ(copyu, testw);
    EXPECT_EQ(copyu, L"test");
    EXPECT_FALSE(testw.isEmpty());
    EXPECT_FALSE(copyu.isEmpty());
}

TEST(CoreAsStr, CreateSstringCopy1) {
    {
        Tstringa sample{"sample"};
        EXPECT_TRUE(sample.isConstantString());
        Tstringa copy1{sample};
        EXPECT_TRUE(copy1.isConstantString());
        EXPECT_TRUE(sample.toStr().isSame(copy1.toStr()));
        EXPECT_EQ(sample.sharedCount(), 0u);
        EXPECT_EQ(copy1.sharedCount(), 0u);
    }
    {
        Tstringa sample{2, "sample"};
        EXPECT_FALSE(sample.isConstantString());
        EXPECT_TRUE(sample.isLocalString());
        Tstringa copy1{sample};
        EXPECT_TRUE(copy1.isLocalString());
        EXPECT_FALSE(sample.toStr().isSame(copy1.toStr()));
        EXPECT_EQ(sample.sharedCount(), 0u);
        EXPECT_EQ(copy1.sharedCount(), 0u);
    }
    {
        Tstringa sample{10, "sample"};
        EXPECT_FALSE(sample.isConstantString());
        EXPECT_FALSE(sample.isLocalString());
        EXPECT_TRUE(sample.isSharedString());
        EXPECT_EQ(sample.sharedCount(), 1u);
        Tstringa copy1{sample};
        EXPECT_TRUE(copy1.isSharedString());
        EXPECT_TRUE(sample.toStr().isSame(copy1.toStr()));
        EXPECT_EQ(sample.sharedCount(), 2u);
        EXPECT_EQ(copy1.sharedCount(), 2u);
    }

    {
        lstringsa<10> sample{10, "sample"};
        EXPECT_FALSE(sample.isEmpty());
        Tstringa copy1{sample};
        EXPECT_FALSE(sample.isEmpty());
        EXPECT_TRUE(copy1.isSharedString());
        EXPECT_EQ(copy1.sharedCount(), 1u);
    }

    {
        lstringsa<10> sample{10, "sample"};
        EXPECT_FALSE(sample.isEmpty());
        Tstringa copy1{std::move(sample)};
        EXPECT_TRUE(sample.isEmpty());
        EXPECT_TRUE(copy1.isSharedString());
        EXPECT_EQ(copy1.sharedCount(), 1u);
    }
}

TEST(CoreAsStr, CreateSstringMove) {

    stringa testa{"test"};
    stringa copya{std::move(testa)};
    EXPECT_NE(copya, testa);
    EXPECT_EQ(copya, "test");
    EXPECT_TRUE(testa.isEmpty());
    EXPECT_FALSE(copya.isEmpty());

    stringu testu{u"test"};
    stringu copyw{std::move(testu)};
    EXPECT_NE(copyw, testu);
    EXPECT_EQ(copyw, u"test");
    EXPECT_TRUE(testu.isEmpty());
    EXPECT_FALSE(copyw.isEmpty());

    stringw testw{L"test"};
    stringw copyu{std::move(testw)};
    EXPECT_NE(copyu, testw);
    EXPECT_EQ(copyu, L"test");
    EXPECT_TRUE(testw.isEmpty());
    EXPECT_FALSE(copyu.isEmpty());
}

TEST(CoreAsStr, CreateSstringChars) {

    stringa testa{4, 'a'};
    EXPECT_EQ(testa, "aaaa");

    stringu testu{4, u'a'};
    EXPECT_EQ(testu, u"aaaa");

    stringw testw{4, L'a'};
    EXPECT_EQ(testw, L"aaaa");
}

TEST(CoreAsStr, CreateSstringSimpleString) {

    ssa sa{"test"};
    stringa testa{sa};
    EXPECT_EQ(testa, "test");

    ssu sw{u"test"};
    stringu testu{sw};
    EXPECT_EQ(testu, u"test");

    ssw su{L"test"};
    stringw testw{su};
    EXPECT_EQ(testw, L"test");
}

TEST(CoreAsStr, CreateSstringSimpleStringRepeat) {

    ssa sa{"test"};
    stringa testa{3, sa};
    EXPECT_EQ(testa, "testtesttest");

    ssu sw{u"test"};
    stringu testu{3, sw};
    EXPECT_EQ(testu, u"testtesttest");

    ssw su{L"test"};
    stringw testw{3, su};
    EXPECT_EQ(testw, L"testtesttest");
}

TEST(CoreAsStr, CreateSstringLong) {
    ssa sa{"test"};
    stringa testa{10, sa};
    EXPECT_EQ(testa, "testtesttesttesttesttesttesttesttesttest");
}

TEST(CoreAsStr, CreateSstringStrExpr) {
    EXPECT_EQ(stringa{eea & "test" & 10 & e_c('a', 3)}, "test10aaa");
    EXPECT_EQ(stringu{eeu & u"test" & 10 & e_c(u'a', 3)}, u"test10aaa");
    EXPECT_EQ(stringw{eew & L"test" & 10 & e_c(L'a', 3)}, L"test10aaa");
}

TEST(CoreAsStr, CreateSstringReplace) {
    EXPECT_EQ(stringa(ssa("tratata"), "t", ""), "raaa");
    EXPECT_EQ(stringa(ssa("tratata"), "t", "-"), "-ra-a-a");
    EXPECT_EQ(stringa(ssa("tratata"), "t", ">>"), ">>ra>>a>>a");

    EXPECT_EQ(stringu(ssu(u"tratata"), u"t", u""), u"raaa");
    EXPECT_EQ(stringu(ssu(u"tratata"), u"t", u"-"), u"-ra-a-a");
    EXPECT_EQ(stringu(ssu(u"tratata"), u"t", u">>"), u">>ra>>a>>a");

    EXPECT_EQ(stringw(ssw(L"tratata"), L"t", L""), L"raaa");
    EXPECT_EQ(stringw(ssw(L"tratata"), L"t", L"-"), L"-ra-a-a");
    EXPECT_EQ(stringw(ssw(L"tratata"), L"t", L">>"), L">>ra>>a>>a");
}

TEST(CoreAsStr, AssignSstring) {
    stringa test{"test"};
    EXPECT_EQ(test = test, "test");
    EXPECT_EQ(test = "next", "next");
    EXPECT_EQ(test = ssa{"other"}, "other");
    EXPECT_EQ(test = stringa{eea & "trtr" & 10}, "trtr10");
    EXPECT_EQ(test = eea & "trtr" & 20, "trtr20");
    EXPECT_EQ(test = test(2), "tr20");
    EXPECT_EQ(test = lstringa<10>{"func"}, "func");
    EXPECT_EQ(test = lstringsa<10>{"func"}, "func");
    lstringsa<10> sample{15, "1234"};
    Tstringa t = sample;
    EXPECT_TRUE(t.isSharedString());
    EXPECT_EQ(t.sharedCount(), 1u);
    EXPECT_FALSE(sample.isEmpty());
    EXPECT_EQ(sample.length(), 60u);
    EXPECT_EQ(sample, t.toStr());

    t = std::move(sample);
    EXPECT_TRUE(t.isSharedString());
    EXPECT_EQ(t.sharedCount(), 1u);
    EXPECT_TRUE(sample.isEmpty());
    EXPECT_EQ(sample.symbols()[0], 0);
    EXPECT_EQ(t.length(), 60u);
}

TEST(CoreAsStr, StrJoin) {
    EXPECT_EQ(stringa::join(std::vector<ssa>{}, "-"), "");
    EXPECT_EQ(stringa::join(std::vector<ssa>{"abc"}, "-"), "abc");
    EXPECT_EQ(stringa::join(std::vector<ssa>{"abc"}, "-", true), "abc-");
    EXPECT_EQ(stringa::join(std::vector<ssa>{"abc", "def", "ghi"}, "-"), "abc-def-ghi");
    EXPECT_EQ(stringa::join(std::vector<ssa>{"abc", "def", "ghi"}, "-", true), "abc-def-ghi-");
}

TEST(CoreAsStr, LStringCreate) {
    lstringa<40> test;
    EXPECT_TRUE(test.isEmpty());
    EXPECT_EQ(test, "");
    EXPECT_EQ(test.length(), 0u);
}

TEST(CoreAsStr, LStringCreateLiteral) {
    lstringa<40> test{"test"};
    EXPECT_FALSE(test.isEmpty());
    EXPECT_EQ(test, "test");
    EXPECT_EQ(test.length(), 4u);
}

TEST(CoreAsStr, LStringCreateSimpleStr) {
    lstringa<40> test{ssa{"test"}};
    EXPECT_FALSE(test.isEmpty());
    EXPECT_EQ(test, "test");
    EXPECT_EQ(test.length(), 4u);
}

TEST(CoreAsStr, LStringCreateCopy) {
    lstringa<40> test{"test"};
    lstringa<40> copy{test};
    EXPECT_EQ(test, copy);
    EXPECT_EQ(copy, "test");
}

TEST(CoreAsStr, LStringCreateMove) {
    lstringa<40> test{"test"};
    lstringa<40> copy{std::move(test)};
    EXPECT_NE(test, copy);
    EXPECT_EQ(copy, "test");
    EXPECT_TRUE(test.isEmpty());
    EXPECT_FALSE(copy.isEmpty());
    EXPECT_EQ(test.symbols()[0], 0);
}

TEST(CoreAsStr, LStringCreateCopyOtherSize) {
    lstringa<10> test{"test"};
    lstringa<40> copy{test};
    EXPECT_EQ(test.toStr(), copy);
    EXPECT_EQ(copy, "test");
}

TEST(CoreAsStr, LStringCreatePad) {
    lstringa<10> test{5, 'a'};
    EXPECT_EQ(test, "aaaaa");
}

TEST(CoreAsStr, LStringCreateExpr) {
    lstringa<40> test{"test" & e_num<u8s>(10) & "-" & 20 & e_spca<3>()};
    EXPECT_FALSE(test.isEmpty());
    EXPECT_EQ(test, "test10-20   ");
}

TEST(CoreAsStr, LStringCreateFunc) {
    EXPECT_EQ(lstringa<20>{[](auto& t) { t = "test";}}, "test");
    EXPECT_EQ(lstringa<20>{[](auto p, size_t l) {
        EXPECT_EQ(l, 20u);
        memcpy(p, "test", std::size("test") - 1);
        return std::size("test") - 1;
    }}, "test");
}

TEST(CoreAsStr, LStringCreateSstring) {
    {
        lstringa<40> test{stringa{"test"}};
        EXPECT_FALSE(test.isEmpty());
        EXPECT_EQ(test, "test");
        EXPECT_EQ(test.length(), 4u);
    }
    {
        stringa t{"test"};
        lstringa<40> test{t};
        EXPECT_FALSE(test.isEmpty());
        EXPECT_FALSE(t.isEmpty());
        EXPECT_EQ(test, "test");
        EXPECT_EQ(test.length(), 4u);
    }
}

TEST(CoreAsStr, LStringAssign) {
    {
        lstringa<40> test{"test"};
        EXPECT_EQ(test, "test");
        test = "";
        EXPECT_EQ(test, "");
        test = "test";
        EXPECT_EQ(test, "test");
    }

    {
        lstringa<40> test{"test"};
        EXPECT_EQ(test, "test");
        const auto& t = test;
        test = t;
        EXPECT_EQ(test, "test");

        test = lstringa<1>{"next step"};
        EXPECT_EQ(test, "next step");

        test = test(0);
        EXPECT_EQ(test, "next step");

        test = test(1, 2);
        EXPECT_EQ(test, "ex");

        test = e_c('a', 100);
        EXPECT_EQ(test.length(), 100u);

        lstringa<40> src{"test"};
        test = std::move(src);
        EXPECT_EQ(test, "test");
        EXPECT_TRUE(src.isEmpty());

        test = e_spca<3>();
        EXPECT_EQ(test, "   ");
    }
}


TEST(CoreAsStr, UtfConvert) {
    EXPECT_EQ(stringa{ssu{u"testпройден"}}, "testпройден");
    EXPECT_EQ(stringa{ssw{L"testпройден"}}, "testпройден");
    EXPECT_EQ(stringa{stringu{u"testпройден"}}, "testпройден");
    EXPECT_EQ(stringa{stringw{L"testпройден"}}, "testпройден");
    EXPECT_EQ(stringa{lstringu<40>{u"testпройден"}}, "testпройден");
    EXPECT_EQ(stringa{lstringw<40>{L"testпройден"}}, "testпройден");

    EXPECT_EQ(stringu{ssa{"testпройден"}}, u"testпройден");
    EXPECT_EQ(stringu{ssw{L"testпройден"}}, u"testпройден");
    EXPECT_EQ(stringu{stringa{"testпройден"}}, u"testпройден");
    EXPECT_EQ(stringu{stringw{L"testпройден"}}, u"testпройден");
    EXPECT_EQ(stringu{lstringa<40>{"testпройден"}}, u"testпройден");
    EXPECT_EQ(stringu{lstringw<40>{L"testпройден"}}, u"testпройден");

    EXPECT_EQ(stringw{ssa{"testпройден"}}, L"testпройден");
    EXPECT_EQ(stringw{ssu{u"testпройден"}}, L"testпройден");
    EXPECT_EQ(stringw{stringa{"testпройден"}}, L"testпройден");
    EXPECT_EQ(stringw{stringu{u"testпройден"}}, L"testпройден");
    EXPECT_EQ(stringw{lstringa<40>{"testпройден"}}, L"testпройден");
    EXPECT_EQ(stringw{lstringu<40>{u"testпройден"}}, L"testпройден");

    EXPECT_EQ(lstringa<10>{ssu{u"testпройден"}}, "testпройден");
    EXPECT_EQ(lstringa<10>{ssw{L"testпройден"}}, "testпройден");
    EXPECT_EQ(lstringa<10>{stringu{u"testпройден"}}, "testпройден");
    EXPECT_EQ(lstringa<10>{stringw{L"testпройден"}}, "testпройден");
    EXPECT_EQ(lstringa<10>{lstringu<40>{u"testпройден"}}, "testпройден");
    EXPECT_EQ(lstringa<10>{lstringw<40>{L"testпройден"}}, "testпройден");

    EXPECT_EQ(lstringu<10>{ssa{"testпройден"}}, u"testпройден");
    EXPECT_EQ(lstringu<10>{ssw{L"testпройден"}}, u"testпройден");
    EXPECT_EQ(lstringu<10>{stringa{"testпройден"}}, u"testпройден");
    EXPECT_EQ(lstringu<10>{stringw{L"testпройден"}}, u"testпройден");
    EXPECT_EQ(lstringu<10>{lstringa<40>{"testпройден"}}, u"testпройден");
    EXPECT_EQ(lstringu<10>{lstringw<40>{L"testпройден"}}, u"testпройден");

    EXPECT_EQ(lstringw<10>{ssa{"testпройден"}}, L"testпройден");
    EXPECT_EQ(lstringw<10>{ssu{u"testпройден"}}, L"testпройден");
    EXPECT_EQ(lstringw<10>{stringa{"testпройден"}}, L"testпройден");
    EXPECT_EQ(lstringw<10>{stringu{u"testпройден"}}, L"testпройден");
    EXPECT_EQ(lstringw<10>{lstringa<40>{"testпройден"}}, L"testпройден");
    EXPECT_EQ(lstringw<10>{lstringu<40>{u"testпройден"}}, L"testпройден");
}

TEST(CoreAsStr, LStrChangeCase) {
    EXPECT_EQ(lstringa<20>{"Test"}.upperOnlyAscii(), "TEST");
    EXPECT_EQ(lstringa<20>{"Test"}.lowerOnlyAscii(), "test");
    EXPECT_EQ(lstringa<20>{"TestПрОвЕрКа"}.upperOnlyAscii(), "TESTПрОвЕрКа");
    EXPECT_EQ(lstringa<20>{"TestПрОвЕрКа"}.lowerOnlyAscii(), "testПрОвЕрКа");
    EXPECT_EQ(lstringa<20>{"TestПрОвЕрКа"}.upper(), "TESTПРОВЕРКА");
    EXPECT_EQ(lstringa<20>{"tesTПрОвЕрКа"}.lower(), "testпроверка");
    EXPECT_EQ(lstringa<20>{"tesTİiȺⱥȾⱦẞßΩФывAsd"}.lower(), "testiiⱥⱥⱦⱦßßωфывasd");
    EXPECT_EQ(lstringa<20>{"testⱢɱⱮɽⱤWas"}.upper(), "TESTⱢⱮⱮⱤⱤWAS");

    EXPECT_EQ(lstringu<20>{u"Test"}.upperOnlyAscii(), u"TEST");
    EXPECT_EQ(lstringu<20>{u"Test"}.lowerOnlyAscii(), u"test");
    EXPECT_EQ(lstringu<20>{u"TestПрОвЕрКа"}.upperOnlyAscii(), u"TESTПрОвЕрКа");
    EXPECT_EQ(lstringu<20>{u"TestПрОвЕрКа"}.lowerOnlyAscii(), u"testПрОвЕрКа");
    EXPECT_EQ(lstringu<20>{u"TestПрОвЕрКа"}.upper(), u"TESTПРОВЕРКА");
    EXPECT_EQ(lstringu<20>{u"tesTПрОвЕрКа"}.lower(), u"testпроверка");
    EXPECT_EQ(lstringu<20>{u"tesTİiȺⱥȾⱦẞßΩФывAsd"}.lower(), u"testiiⱥⱥⱦⱦßßωфывasd");
    EXPECT_EQ(lstringu<20>{u"testⱢɱⱮɽⱤWas"}.upper(), u"TESTⱢⱮⱮⱤⱤWAS");
}

TEST(CoreAsStr, LStrTrim) {
    EXPECT_EQ(lstringa<20>{"testing"}.trim(), "testing");
    EXPECT_EQ(lstringa<20>{"  \ttesting  "}.trim(), "testing");
    EXPECT_EQ(lstringa<20>{"  \ttesting  "}.trimLeft(), "testing  ");
    EXPECT_EQ(lstringa<20>{"  \ttesting  "}.trimRight(), "  \ttesting");
    EXPECT_EQ(lstringa<20>{"testing"}.trim("tg"), "estin");
    EXPECT_EQ(lstringa<20>{"testing"}.trimLeft("tg"), "esting");
    EXPECT_EQ(lstringa<20>{"testing"}.trimRight("tg"), "testin");
    EXPECT_EQ(lstringa<20>{"  \ttesting  "}.trimWithSpaces("tg"), "estin");
    EXPECT_EQ(lstringa<20>{"  \ttesting  "}.trimLeftWithSpaces("tg"), "esting  ");
    EXPECT_EQ(lstringa<20>{"  \ttesting  "}.trimRightWithSpaces("tg"), "  \ttestin");
    EXPECT_EQ(lstringa<20>{"testing"}.trim(ssa{"tg"}), "estin");
    EXPECT_EQ(lstringa<20>{"testing"}.trimLeft(ssa{"tg"}), "esting");
    EXPECT_EQ(lstringa<20>{"testing"}.trimRight(ssa{"tg"}), "testin");
    EXPECT_EQ(lstringa<20>{"  \ttesting  "}.trimWithSpaces(ssa{"tg"}), "estin");
    EXPECT_EQ(lstringa<20>{"  \ttesting  "}.trimLeftWithSpaces(ssa{"tg"}), "esting  ");
    EXPECT_EQ(lstringa<20>{"  \ttesting  "}.trimRightWithSpaces(ssa{"tg"}), "  \ttestin");
}

TEST(CoreAsStr, LStrAppend) {
    EXPECT_EQ(lstringa<20>{"test"}.append("ing"), "testing");
    EXPECT_EQ(lstringa<20>{"test"}.append(eea & "ing" & 10), "testing10");
    EXPECT_EQ(lstringa<20>{"test"}.appendIn(3, "ing"), "tesing");
    EXPECT_EQ(lstringa<20>{"test"}.appendIn(3, eea & "ing" & 10), "tesing10");
    EXPECT_EQ(lstringa<20>{"test"} += "ing", "testing");
    EXPECT_EQ(lstringa<20>{"test"} += eea & "ing" & 10, "testing10");
}

TEST(CoreAsStr, LStrChange) {
    EXPECT_EQ(lstringa<20>{"test"}.insert(1, "---"), "t---est");
    EXPECT_EQ(lstringa<20>{"test"}.insert(1, eea & "---"), "t---est");
    EXPECT_EQ(lstringa<20>{"test"}.insert(100, "---"), "test---");
    EXPECT_EQ(lstringa<20>{"test"}.prepend("---"), "---test");
    EXPECT_EQ(lstringa<3>{"test"}.prepend(eea & "---"), "---test");
    EXPECT_EQ(lstringa<4>{"test"}.change(1, 2, "---"), "t---t");
    EXPECT_EQ(lstringa<20>{"test"}.change(1, 2, eea & "---"), "t---t");
    EXPECT_EQ(lstringa<0>{"test"}.change(100, 20, "---"), "test---");
    EXPECT_EQ(lstringa<1>{"test"}.remove(1, 2), "tt");
    EXPECT_EQ(lstringa<20>{""}.change(1, 5, "---"), "---");

    EXPECT_EQ(lstringu<20>{u"test"}.insert(1, u"---"), u"t---est");
    EXPECT_EQ(lstringu<20>{u"test"}.insert(1, eeu & u"---"), u"t---est");
    EXPECT_EQ(lstringu<20>{u"test"}.insert(100, u"---"), u"test---");
    EXPECT_EQ(lstringu<20>{u"test"}.prepend(u"---"), u"---test");
    EXPECT_EQ(lstringu<3>{u"test"}.prepend(eeu & u"---"), u"---test");
    EXPECT_EQ(lstringu<4>{u"test"}.change(1, 2, u"---"), u"t---t");
    EXPECT_EQ(lstringu<20>{u"test"}.change(1, 2, eeu & u"---"), u"t---t");
    EXPECT_EQ(lstringu<0>{u"test"}.change(100, 20, u"---"), u"test---");
    EXPECT_EQ(lstringu<1>{u"test"}.remove(1, 2), u"tt");
    EXPECT_EQ(lstringu<20>{u""}.change(1, 5, u"---"), u"---");
}

TEST(CoreAsStr, LStrSelfReplace) {
    {
        lstringa<40> test = "test string";
        ssa before = test;
        test.replace("st", "asd");
        EXPECT_EQ(test, "teasd asdring");
        EXPECT_EQ(before.str, test.toStr().str);
    }
    {
        lstringa<40> test = "test string";
        ssa before = test;
        test.replace("st", "a");
        EXPECT_EQ(test, "tea aring");
        EXPECT_EQ(before.str, test.toStr().str);
    }

    {
        lstringa<40> test = "test string";
        ssa before = test;
        test.replace("st", "--");
        EXPECT_EQ(test, "te-- --ring");
        EXPECT_EQ(before.str, test.toStr().str);
    }

    {
        lstringa<11> test = "test string";
        ssa before = test;
        test.replace("st", "---");
        EXPECT_EQ(test, "te--- ---ring");
        // buffer changed from local to allocated
        EXPECT_NE(before.str, test.toStr().str);
        test.replace("---", "s");
        EXPECT_EQ(test, "tes sring");
        // buffer changed from allocated to local
        EXPECT_EQ(before.str, test.toStr().str);
    }

    {
        lstringa<40> buffer;
        buffer.replaceFrom(ssa{"test string"}, "st", "---");
        EXPECT_EQ(buffer, "te--- ---ring");
    }
}

TEST(CoreAsStr, LStrSelfFuncFill) {
    size_t count = 0, first = 0, second = 0;
    lstringa<5> test;
    test << [&](auto p, auto l) {
        if (!count) {
            first = l;
        } else {
            second = l;
        }
        count++;
        if (l >= 10) {
            memset(p, 'a', 10);
        }
        return 10;
    };
    EXPECT_EQ(count, 2);
    EXPECT_EQ(first, 5);
    EXPECT_EQ(second, 10);
    EXPECT_EQ(test, "aaaaaaaaaa");

    count = 0, first = 0, second = 0;
    test <<= [&](auto p, auto l) {
        if (!count) {
            first = l;
        } else {
            second = l;
        }
        count++;
        if (l >= 5) {
            memset(p, 'b', 5);
        }
        return 5;
    };
    EXPECT_EQ(count, 2u);
    EXPECT_EQ(first, 0u);
    EXPECT_EQ(second, 5u);
    EXPECT_EQ(test, "aaaaaaaaaabbbbb");
}

TEST(CoreAsStr, SStrTrimCopy) {
    stringa sample{"test"};
    stringa result = sample.trimmed();
    EXPECT_EQ(sample, result);
    EXPECT_EQ(sample.symbols(), result.symbols());
    stringa yares = sample.trimmedRight("t");
    EXPECT_EQ(yares, "tes");
    EXPECT_NE(sample.symbols(), yares.symbols());

    auto ls = sample.trimmed<lstringa<20>>();
    EXPECT_EQ(ls, "test");
}

TEST(CoreAsStr, LStrPrintf) {
    {
        lstringa<2> text;
        text.printf("%c%c", 'a', 'b');
        EXPECT_EQ(text, "ab");
        text.printf("%s", "tested");
        EXPECT_EQ(text, "tested");
    }
    {
        lstringw<2> text;
        text.printf(L"%c%c", 'a', 'b');
        EXPECT_EQ(text, L"ab");
        text.printf(L"%ls", L"tested");
        EXPECT_EQ(text, L"tested");
    }
    {
        lstringa<40> text = "tested";
        text.printfFrom(4, "%c%c", 'a', 'b');
        EXPECT_EQ(text, "testab");
        text.printfFrom(1, "%s", "tested");
        EXPECT_EQ(text, "ttested");
    }

    {
        lstringa<40> text = "tested";
        text.appendPrintf("%c%c", 'a', 'b');
        EXPECT_EQ(text, "testedab");
        text.appendPrintf("%s", "tested");
        EXPECT_EQ(text, "testedabtested");
    }
}

TEST(CoreAsStr, LStrFormat) {
    {
        lstringa<2> text;
        text.format("{}{}", 'a', 'b');
        EXPECT_EQ(text, "ab");
        text.format("{}", "tested");
        EXPECT_EQ(text, "tested");
    }
    {
        lstringw<2> text;
        text.format(L"{}{}", 'a', 'b');
        EXPECT_EQ(text, L"ab");
        text.format(L"{}", L"tested");
        EXPECT_EQ(text, L"tested");
    }
    {
        lstringa<40> text = "tested";
        text.formatFrom(4, "{}{}", 'a', 'b');
        EXPECT_EQ(text, "testab");
        text.formatFrom(1, "{}", "tested");
        EXPECT_EQ(text, "ttested");
    }

    {
        lstringa<40> text = "tested";
        text.appendFormated("{}{}", 'a', 'b');
        EXPECT_EQ(text, "testedab");
        text.appendFormated("{}", "tested");
        EXPECT_EQ(text, "testedabtested");
    }
}

TEST(CoreAsStr, LStrFormatExpressions) {
    lstringa<100> buffer;
    buffer = e_ls<true>(std::vector<ssa>{}, "<>");
    EXPECT_EQ(buffer, "");
    buffer = e_ls<true>(std::vector<ssa>{""}, "<>");
    EXPECT_EQ(buffer, "<>");

    buffer = e_ls(std::vector<ssa>{"asd", "fgh", "jkl"}, "<>");
    EXPECT_EQ(buffer, "asd<>fgh<>jkl");

    int k = 10;
    double d = 12.1;
    lstringa<10> test = ">" & e_repl(buffer.toStr(), "<>", "-") & k & ", " & d;
    EXPECT_EQ(test, ">asd-fgh-jkl10, 12.1");

    buffer.prepend(e_choice(test.length() > 2, eea & 99, eea & 12.1 & "asd"));
    EXPECT_EQ(buffer, "99asd<>fgh<>jkl");

    buffer.change(2, 4, test(0, 3) & "__" & 1 & ',');
    EXPECT_EQ(buffer, "99>as__1,>fgh<>jkl");
}

TEST(CoreAsStr, LStrVFormat){
    ssa t1 = "text1", t2 = "text2";
    auto res = lstringa<4>{}.vformat("{}{}{}", t1, t2, 4);
    EXPECT_EQ(res, "text1text24");
}

TEST(CoreAsStr, SStrFormat) {
    EXPECT_EQ(stringa::printf("%s%i", "test", 10), "test10");
    EXPECT_EQ(stringa::format("{}{}", "test", 10), "test10");
    EXPECT_EQ(stringw::printf(L"%ls%i", L"test", 10), L"test10");
    EXPECT_EQ(stringw::format(L"{}{}", L"test", 10), L"test10");
}

#define _S(par) par, size_t(std::size(par) - 1)

TEST(CoreAsStr, HashMap) {
    EXPECT_EQ(fnv_hash( "asdfghjkl"), fnv_hash(_S( "asdfghjkl")));
    EXPECT_EQ(fnv_hash(u"asdfghjkl"), fnv_hash(_S(u"asdfghjkl")));
    EXPECT_EQ(fnv_hash(L"asdfghjkl"), fnv_hash(_S(L"asdfghjkl")));

    EXPECT_EQ(fnv_hash( "asdfghjkl"), fnv_hash_compile(_S( "asdfghjkl")));
    EXPECT_EQ(fnv_hash(u"asdfghjkl"), fnv_hash_compile(_S(u"asdfghjkl")));
    EXPECT_EQ(fnv_hash(L"asdfghjkl"), fnv_hash_compile(_S(L"asdfghjkl")));

    EXPECT_EQ(fnv_hash_ia( "asdfGhjkl"), fnv_hash_ia(_S( "asDFghjkl")));
    EXPECT_EQ(fnv_hash_ia(u"asdfghJkl"), fnv_hash_ia(_S(u"asdFGHjkl")));
    EXPECT_EQ(fnv_hash_ia(L"Asdfghjkl"), fnv_hash_ia(_S(L"asdfghjKL")));
    
    EXPECT_EQ(fnv_hash_ia( "asDfghjkl"), fnv_hash_ia_compile(_S( "aSDfghjkl")));
    EXPECT_EQ(fnv_hash_ia(u"asdFghjkl"), fnv_hash_ia_compile(_S(u"asdFGhjkl")));
    EXPECT_EQ(fnv_hash_ia(L"asDfghjkl"), fnv_hash_ia_compile(_S(u"asdFGhjkl")));

    EXPECT_EQ(fnv_hash_ia( "asdfGhjkl"), unicode_traits<u8s> ::hashia(_S( "asDFghjkl")));
    EXPECT_EQ(fnv_hash_ia(u"asdfghJkl"), unicode_traits<u16s>::hashia(_S(u"asDFghjkl")));
    EXPECT_EQ(fnv_hash_ia(U"asDFghJkl"), unicode_traits<u32s>::hashia(_S(U"asdFGhjkl")));

    EXPECT_EQ(unicode_traits<u8s>:: hashia(_S( "asDFghjkl")),
              unicode_traits<u8s>:: hashia(_S( "Asdfghjkl")));
    EXPECT_EQ(unicode_traits<u16s>::hashia(_S(u"asdFGhjkl")),
              unicode_traits<u16s>::hashia(_S(u"aSDfghjkl")));
    EXPECT_EQ(unicode_traits<u32s>::hashia(_S(U"asdfGhjkl")),
              unicode_traits<u32s>::hashia(_S(U"asDfghjkl")));

    EXPECT_EQ(unicode_traits<u8s>:: hashiu(_S( "asDFghjklРус")),
              unicode_traits<u8s>:: hashiu(_S( "AsdfghjklрУС")));
    EXPECT_EQ(unicode_traits<u16s>::hashiu(_S(u"asdFGhjklРус")),
              unicode_traits<u16s>::hashiu(_S(u"aSDfghjklрУС")));
    EXPECT_EQ(unicode_traits<u32s>::hashiu(_S(U"asdfGhjklРус")),
              unicode_traits<u32s>::hashiu(_S(U"asDfghjklрУС")));

    {
        hashStrMapA<int> test = {
            {"asd"_h, 1},
            {"fgh"_h, 2},
        };
        
        EXPECT_EQ(test.find("aaa"), test.end());
        EXPECT_NE(test.find("asd"_h), test.end());
        EXPECT_EQ(test.find("asd")->second, 1);
    }

    {
        hashStrMapAIA<int> test = {
            {"asd"_ia, 1},
            {"fgh"_ia, 2},
            {"rusрус"_ia, 3},
        };
        
        EXPECT_EQ(test.find("aaa"), test.end());
        EXPECT_EQ(test.find("RUSРУС"_ia), test.end());
        EXPECT_EQ(test.find("aSd")->second, 1);
    }

    {
        hashStrMapAIA<int> test = {
    #ifdef _MSC_VER
            std::initializer_list<std::pair<const stringa, int>>{
    #endif
            {"asd", 1},
            {"fgh", 2},
            {"rusрус", 3},

    #ifdef _MSC_VER
            }
    #endif
        };

        
        EXPECT_EQ(test.find("aaa"), test.end());
        EXPECT_EQ(test.find("RUSРУС"), test.end());
        EXPECT_EQ(test.find("aSd"_ia)->second, 1);
    }

    {
        hashStrMapAIU<int> test = {
            {"asd"_iu, 1},
            {"fgh"_iu, 2},
            {"rusрус"_iu, 3},
        };
        
        EXPECT_EQ(test.find("aaa"), test.end());
        EXPECT_NE(test.find("RUSРУС"), test.end());
        EXPECT_EQ(test.find("RUSРУС"_iu)->second, 3);
    }
}

} // namespace core_as::str::tests
