#include "format.hpp"

#include "converter.hpp"
#include "locales.hpp"
#include "user_defined.hpp"

#include <catch.hpp>

TEMPLATE_TEST_CASE("locale_std_examples", "", char, wchar_t) {
    using lrstd::format;

    str_fn<TestType> str;

    CHECK(format(std::locale("C"), str("{0}-{{"), 8) == str("8-{"));

    CHECK(format(std::locale("C"), str("{} to {}"), str("a"), str("b")) ==
          str("a to b"));
    CHECK(format(std::locale("C"), str("{1} to {0}"), str("a"), str("b")) ==
          str("b to a"));

    CHECK_THROWS_AS(
          format(std::locale("C"), str("{0} to {}"), str("a"), str("b")),
          lrstd::format_error);
    CHECK_THROWS_AS(
          format(std::locale("C"), str("{} to {1}"), str("a"), str("b")),
          lrstd::format_error);

    {
        TestType c = 120;
        CHECK(format(std::locale("C"), str("{:6}"), 42) == str("    42"));
        CHECK(format(std::locale("C"), str("{:6}"), str('x')) == str("x     "));
        CHECK(format(std::locale("C"), str("{:*<6}"), str('x')) ==
              str("x*****"));
        CHECK(format(std::locale("C"), str("{:*>6}"), str('x')) ==
              str("*****x"));
        CHECK(format(std::locale("C"), str("{:*^6}"), str('x')) ==
              str("**x***"));
        CHECK(format(std::locale("C"), str("{:*^{}}"), str('x'), 6) ==
              str("**x***"));
        CHECK(format(std::locale("C"), str("{:6d}"), c) == str("   120"));
        CHECK(format(std::locale("C"), str("{:6}"), true) == str("true  "));
    }

    {
          // clang-format off

        // TODO
        // double inf = std::numeric_limits<double>::infinity();
        // double nan = std::numeric_limits<double>::quiet_NaN();
        // CHECK(format(std::locale("C" ), "{0:},{0:+},{0:-},{0: }", 1) == "1,+1,1, 1");
        // CHECK(format(std::locale("C" ), "{0:},{0:+},{0:-},{0: }", -1) == "-1,-1,-1,-1");
        // CHECK(format(std::locale("C" ), "{0:},{0:+},{0:-},{0: }", inf) == "inf,+inf,inf, inf");
        // CHECK(format(std::locale("C" ), "{0:},{0:+},{0:-},{0: }", nan) == "nan,+nan,nan, nan");

          // clang-format on
    }

    {
        TestType c = 120;
        CHECK(format(std::locale("C"), str("{:+06d}"), c) == str("+00120"));
        CHECK(format(std::locale("C"), str("{:#06x}"), 0xa) == str("0x000a"));
        CHECK(format(std::locale("C"), str("{:<06}"), -42) == str("-42   "));
    }

    {
        CHECK(format(std::locale("C"), str("{}"), 42) == str("42"));
        CHECK(format(std::locale("C"), str("{0:b} {0:d} {0:o} {0:x}"), 42) ==
              str("101010 42 52 2a"));
        CHECK(format(std::locale("C"), str("{0:#x} {0:#X}"), 42) ==
              str("0x2a 0X2A"));

        // clang-format off
        // TODO
        // CHECK(format(std::locale("C" ), "{:L}", 1234) == "1,234");  // (depending on the locale)
        // clang-format on
    }
    { CHECK(format(std::locale("C"), str("{}"), red) == str("red")); }
    {
        CHECK(format(std::locale("C"), str("{0:{1}}"), S{42}, 10) ==
              str("xxxxxxxx42"));
    }
}

TEMPLATE_TEST_CASE("locale_format_to_n", "", char, wchar_t) {
    using namespace std::string_view_literals;
    using namespace lrstd;

    str_fn<TestType> str;
    SV<TestType> sv;

    {
        std::array<TestType, 64> chars{};
        {
            auto result = format_to_n(chars.begin(), 11, std::locale("C"),
                                      str("unavoidable paradigm"));
            CHECK(result.size == 20);
            CHECK(result.out == std::next(chars.begin(), 11));
            CHECK(sv(chars) == str("unavoidable"));
        }
        {
            auto result = format_to_n(chars.begin(), 16, std::locale("C"),
                                      str("the system is {1} free from {0}"),
                                      str("error"), str("now"));
            CHECK(result.size == 33);
            CHECK(result.out == std::next(chars.begin(), 16));
            CHECK(sv(chars) == str("the system is no"));
        }
    }
    {
        std::vector<TestType> chars;
        {
            auto result =
                  format_to_n(std::back_inserter(chars), 10000000,
                              std::locale("C"), str("the {}"), str("system"));
            CHECK(result.size == 10);
            CHECK(sv(chars) == str("the system"));
        }
        {
            auto result =
                  format_to_n(std::back_inserter(chars), 0, std::locale("C"),
                              str(" revokes our {} form"), str("viral"));
            CHECK(result.size == 23);
            CHECK(sv(chars) == str("the system"));
        }
    }
    {
        std::basic_string<TestType> chars;
        auto result = format_to_n(std::back_inserter(chars), 20,
                                  std::locale("C"), str("{:*^20}"), str(""));
        CHECK(chars == str("********************"));
        CHECK(result.size == 20);
        chars.clear();
        result = format_to_n(std::back_inserter(chars), 10, std::locale("C"),
                             str("{:*>20}"), str(""));
        CHECK(chars == str("**********"));
        CHECK(result.size == 20);
    }
    {
        std::basic_string<TestType> chars;
        auto result =
              format_to_n(std::back_inserter(chars), 200, std::locale("C"),
                          str("{0:_>5}{0:-^3}{0:_<5}"), str('a'));
        CHECK(chars == str("____a-a-a____"));
        CHECK(result.size == 13);
        chars.clear();
        result = format_to_n(std::back_inserter(chars), 8, std::locale("C"),
                             str("{0:_>5}{0:-^3}{0:_<5}"), str('a'));
        CHECK(chars == str("____a-a-"));
        CHECK(result.size == 13);
    }
}

TEMPLATE_TEST_CASE("locale_formatted_size", "", char, wchar_t) {
    using namespace std::literals;
    using lrstd::formatted_size;

    str_fn<TestType> str;

    {
        CHECK(formatted_size(std::locale("C"), str("{}"), str("hey")) == 3);
        CHECK(formatted_size(std::locale("C"), str("{0: ^6}"), str("hey")) ==
              6);
        CHECK(formatted_size(std::locale("C"), str("{:_>6}"), str("hey")) == 6);
    }
    {
        CHECK(formatted_size(std::locale("C"), str("{:s}"), true) == 4);
        CHECK(formatted_size(std::locale("C"), str("{:s}"), false) == 5);
        CHECK(formatted_size(std::locale("C"), str("{0: ^6s}"), true) == 6);
        CHECK(formatted_size(std::locale("C"), str("{:_>6s}"), false) == 6);
    }

    CHECK(formatted_size(std::locale("C"), str("{0}-{{"), 8) == 3);

    CHECK(formatted_size(std::locale("C"), str("{} to {}"), str("a"),
                         str("b")) == 6);
    CHECK(formatted_size(std::locale("C"), str("{1} to {0}"), str("a"),
                         str("b")) == 6);

    {
        char c = 120;
        CHECK(formatted_size(std::locale("C"), str("{:6}"), 42) == 6);
        CHECK(formatted_size(std::locale("C"), str("{:6}"), str('x')) == 6);
        CHECK(formatted_size(std::locale("C"), str("{:*<6}"), str('x')) == 6);
        CHECK(formatted_size(std::locale("C"), str("{:*>6}"), str('x')) == 6);
        CHECK(formatted_size(std::locale("C"), str("{:*^6}"), str('x')) == 6);
        CHECK(formatted_size(std::locale("C"), str("{:*^{}}"), str('x'), 6) ==
              6);
        CHECK(formatted_size(std::locale("C"), str("{:6d}"), c) == 6);
        CHECK(formatted_size(std::locale("C"), str("{:6}"), true) == 6);
    }

    {
          // clang-format off

        // TODO
        // double inf = std::numeric_limits<double>::infinity();
        // double nan = std::numeric_limits<double>::quiet_NaN();
        // CHECK(format("{0:},{0:+},{0:-},{0: }", 1) == "1,+1,1, 1");
        // CHECK(format("{0:},{0:+},{0:-},{0: }", -1) == "-1,-1,-1,-1");
        // CHECK(format("{0:},{0:+},{0:-},{0: }", inf) == "inf,+inf,inf, inf");
        // CHECK(format("{0:},{0:+},{0:-},{0: }", nan) == "nan,+nan,nan, nan");

          // clang-format on
    }

    {
        TestType c = 120;
        CHECK(formatted_size(std::locale("C"), str("{:+06d}"), c) == 6);
        CHECK(formatted_size(std::locale("C"), str("{:#06x}"), 0xa) == 6);
        CHECK(formatted_size(std::locale("C"), str("{:<06}"), -42) == 6);
    }

    {
        CHECK(formatted_size(std::locale("C"), str("{}"), 42) == 2);
        CHECK(formatted_size(std::locale("C"), str("{0:b} {0:d} {0:o} {0:x}"),
                             42) == 15);
        CHECK(formatted_size(std::locale("C"), str("{0:#x} {0:#X}"), 42) == 9);

        // clang-format off
        // TODO
        // CHECK(format("{:L}", 1234) == "1,234");  // (depending on the locale)
        // clang-format on
    }
    { CHECK(formatted_size(std::locale("C"), str("{}"), red) == 3); }
    {
        CHECK(formatted_size(std::locale("C"), str("{0:{1}}"), S{42}, 10) ==
              10);
    }
}

TEMPLATE_TEST_CASE("int_locale",
                   "",
                   (std::tuple<char, en_US_locale>),
                   (std::tuple<wchar_t, en_US_locale>),
                   (std::tuple<char, en_US_funky_locale>),
                   (std::tuple<wchar_t, en_US_funky_locale>)) {
    using lrstd::format;
    using charT = std::tuple_element_t<0, TestType>;
    str_fn<charT> str;

    using loc_getter = std::tuple_element_t<1, TestType>;
    std::locale loc = loc_getter{}();

    CHECK(format(loc, str("{:L}"), 1234) == str("1,234"));
    CHECK(format(loc, str("{}"), 1234) == str("1234"));
    CHECK(format(loc, str("{:}"), 1234) == str("1234"));
    CHECK(format(loc, str("{:d}"), 1234) == str("1234"));

    CHECK(format(loc, str("{:L}"), -1234) == str("-1,234"));
    CHECK(format(loc, str("{}"), -1234) == str("-1234"));
    CHECK(format(loc, str("{:}"), -1234) == str("-1234"));
    CHECK(format(loc, str("{:d}"), -1234) == str("-1234"));

    CHECK(format(loc, str("{:<L}"), -1234) == str("-1,234"));
    CHECK(format(loc, str("{:>L}"), -1234) == str("-1,234"));
    CHECK(format(loc, str("{:^L}"), -1234) == str("-1,234"));
    CHECK(format(loc, str("{:0L}"), -1234) == str("-1,234"));

    CHECK(format(loc, str("{:<8L}"), -1234) == str("-1,234  "));
    CHECK(format(loc, str("{:>8L}"), -1234) == str("  -1,234"));
    CHECK(format(loc, str("{:^8L}"), -1234) == str(" -1,234 "));
    CHECK(format(loc, str("{:08L}"), -1234) == str("-001,234"));

    CHECK(format(loc, str("{:<+8L}"), 1234) == str("+1,234  "));
    CHECK(format(loc, str("{:>+8L}"), 1234) == str("  +1,234"));
    CHECK(format(loc, str("{:^+8L}"), 1234) == str(" +1,234 "));
    CHECK(format(loc, str("{:+08L}"), 1234) == str("+001,234"));

    CHECK(format(loc, str("{:< 8L}"), 1234) == str(" 1,234  "));
    CHECK(format(loc, str("{:> 8L}"), 1234) == str("   1,234"));
    CHECK(format(loc, str("{:^ 8L}"), 1234) == str("  1,234 "));
    CHECK(format(loc, str("{: 08L}"), 1234) == str(" 001,234"));

    CHECK(format(loc, str("{:Lx}"), 10000) == str("2,710"));
    CHECK(format(loc, str("{:Lx}"), 10000) == str("2,710"));
    CHECK(format(loc, str("{:Lb}"), 10000) == str("10,011,100,010,000"));
}

TEMPLATE_TEST_CASE("int_cppreference_examples", "", char, wchar_t) {
    using lrstd::format;
    using charT = TestType;
    str_fn<charT> str;

    struct space_out : std::numpunct<charT> {
        charT do_thousands_sep() const override {
            return ' ';
        }  // separate with spaces
        std::string do_grouping() const override {
            return "\1";
        }  // groups of 1 digit
    };
    {
        std::locale loc(std::locale("C"), new space_out);
        CHECK(format(loc, str("{:L}"), 12345678) == str("1 2 3 4 5 6 7 8"));
        CHECK(format(loc, str("{:*^19L}"), 12345678) ==
              str("**1 2 3 4 5 6 7 8**"));
    }

    struct g123 : std::numpunct<charT> {
        std::string do_grouping() const override { return "\1\2\3"; }
    };
    {
        std::locale loc(std::locale("C"), new g123);
        CHECK(format(loc, str("{:L}"),
                     std::numeric_limits<unsigned long long>::max()) ==
              str("18,446,744,073,709,551,61,5"));
        CHECK(format(loc, str("{:30L}"),
                     std::numeric_limits<unsigned long long>::max()) ==
              str("   18,446,744,073,709,551,61,5"));

        CHECK(format(loc, str("{:L}"), 15) == str("1,5"));
        CHECK(format(loc, str("{:^5L}"), 15) == str(" 1,5 "));

        CHECK(format(loc, str("{:L}"), 615) == str("61,5"));
        CHECK(format(loc, str("{:^5L}"), 615) == str("61,5 "));

        CHECK(format(loc, str("{:L}"), 1615) == str("1,61,5"));
        CHECK(format(loc, str("{:^8L}"), 1615) == str(" 1,61,5 "));
    }
}

TEMPLATE_TEST_CASE("bool_locale", "", char, wchar_t) {
    using lrstd::format;
    using charT = TestType;
    str_fn<charT> str;
    {
        std::locale::global(std::locale("en_US"));
        CHECK(format(str("{:L}"), true) == str("true"));
        CHECK(format(str("{:L}"), true) == str("true"));
    }
    {
        CHECK(format(std::locale("en_US"), str("{:L}"), true) == str("true"));
        CHECK(format(std::locale("en_US"), str("{:L}"), true) == str("true"));
    }
}

TEMPLATE_TEST_CASE("bool_cppreference_examples", "", char, wchar_t) {
    using lrstd::format;
    using charT = TestType;
    str_fn<charT> str;

    {
        std::locale::global(std::locale("en_US"));
        CHECK(format(str("{:L}"), true) == str("true"));
        CHECK(format(str("{:L}"), true) == str("true"));
    }
    {
        std::locale loc("C");
        CHECK(format(loc, str("{:L}"), true) == str("true"));
        CHECK(format(loc, str("{:L}"), false) == str("false"));
    }
    {
        struct custom_tf : std::numpunct<charT> {
            typename std::numpunct<charT>::string_type do_truename()
                  const override {
                return str_fn<charT>{}("t");
            }
            typename std::numpunct<charT>::string_type do_falsename()
                  const override {
                return str_fn<charT>{}("f");
            }
        };
        std::locale loc(std::locale("C"), new custom_tf);
        CHECK(format(loc, str("{:L}"), true) == str("t"));
        CHECK(format(loc, str("{:L}"), false) == str("f"));
        CHECK(format(loc, str("{:>5L}"), true) == str("    t"));
        CHECK(format(loc, str("{:>5L}"), false) == str("    f"));
    }
    {
        struct french_bool : std::numpunct<charT> {
            typename std::numpunct<charT>::string_type do_truename()
                  const override {
                return str_fn<charT>{}("vrai");
            }
            typename std::numpunct<charT>::string_type do_falsename()
                  const override {
                return str_fn<charT>{}("faux");
            }
        };
        std::locale loc(std::locale("C"), new french_bool);
        CHECK(format(loc, str("{:L}"), true) == str("vrai"));
        CHECK(format(loc, str("{:L}"), false) == str("faux"));
        CHECK(format(loc, str("{:>5L}"), true) == str(" vrai"));
        CHECK(format(loc, str("{:>5L}"), false) == str(" faux"));
    }
}
