// Copyright (c) 2020 by the Zeek Project. See LICENSE for details.

#include <string>

#include <hilti/rt/doctest.h>
#include <hilti/rt/fmt.h>
#include <hilti/rt/types/integer.h>
#include <hilti/rt/types/map.h>

using namespace hilti::rt;

TEST_SUITE_BEGIN("Map");

TEST_CASE("get") {
    Map<int, int> m;
    DOCTEST_CHECK_THROWS_WITH_AS(m.get(1), "key is unset", const IndexError&);

    m[1] = 2;
    CHECK_EQ(m.get(1), 2);
}

TEST_CASE("subscript") {
    SUBCASE("rvalue") {
        using M = Map<int, int>;
        CHECK_THROWS_WITH_AS(M()[99], "key is unset", const IndexError&);
    }

    SUBCASE("const lvalue") {
        const Map<int, int> m;
        CHECK_THROWS_WITH_AS(m[99], "key is unset", const IndexError&);
    }

    SUBCASE("mut lvalue") {
        Map<int, int> m;

        m[1] = 11;
        CHECK(m.contains(1));
        CHECK_EQ(m[1].operator int(), 11);
        int m1 = m[1];
        CHECK_EQ(m1, 11);
        CHECK_THROWS_WITH_AS(m[99].operator int(), "key is unset", const IndexError&);

        // Proxy objects perform lazy insertion.
        auto p = m[2];
        (void)p;
        CHECK(! m.contains(2));

        // Proxy objects stringify to the referenced value if it exists, and throw otherwise.
        CHECK_EQ(to_string(m[1]), "11");
        CHECK_THROWS_WITH_AS(to_string(m[2]), "key is unset", const IndexError&);

        CHECK_EQ(fmt("%s", m[1]), "11");

        // Proxy objects only invalidate iterators if an element was actually inserted.
        m = Map<int, int>{{1, 11}};
        REQUIRE(m.size());
        auto begin = m.begin();
        REQUIRE_EQ(begin->first, 1);
        REQUIRE_EQ(begin->second, 11);

        // Just modify existing entry.
        m[1] = 111;
        CHECK_EQ(begin->first, 1);
        CHECK_EQ(begin->second, 111);

        // Invalidating insertion of new entry.
        m[2] = 22;
        REQUIRE(m.contains(2));
        CHECK_THROWS_WITH_AS(*begin, "iterator is invalid", const IndexError&);
    }
}

TEST_CASE("contains") {
    Map<int, std::string> m({{1, "1"}, {2, "2"}});
    CHECK(m.contains(1));
    CHECK(m.contains(2));
    CHECK(! m.contains(99));
}

TEST_CASE("Iterator") {
    SUBCASE("equality") {
        Map<int, std::string> m1({{1, "1"}});

        CHECK_EQ(m1.begin(), m1.begin());
        CHECK_EQ(m1.end(), m1.end());
        CHECK_NE(m1.begin(), m1.end());

        Map<int, std::string> m2({{1, "1"}});
        CHECK_THROWS_WITH_AS(operator==(m1.begin(), m2.begin()), "cannot compare iterators into different maps",
                             const InvalidArgument&);
    }

    SUBCASE("clear") {
        Map<int, std::string> m({{1, "1"}});

        auto begin = m.begin();
        REQUIRE_EQ(begin->first, 1);

        // `clear` invalidates all iterators.
        m.clear();
        CHECK_THROWS_WITH_AS(*begin, "iterator is invalid", const IndexError&);
        CHECK_THROWS_WITH_AS(++begin, "iterator is invalid", const IndexError&);
    }

    SUBCASE("erase") {
        Map<int, std::string> m({{1, "1"}, {2, "2"}});

        auto it1 = m.begin();
        REQUIRE_EQ(it1->first, 1);

        auto it2 = ++m.begin();
        REQUIRE_EQ(it2->first, 2);

        // Erasing a non-existing key does not invalidate iterators.
        m.erase(99);

        REQUIRE_EQ(it1->first, 1);
        REQUIRE_EQ(it2->first, 2);

        // `erase` invalidates all iterators if an element is removed, not just
        // the iterator to the erased element.
        CHECK_EQ(m.erase(1), 1);
        CHECK_THROWS_WITH_AS(*it1, "iterator is invalid", const IndexError&);
        CHECK_THROWS_WITH_AS(*it2, "iterator is invalid", const IndexError&);
    }

    SUBCASE("increment") {
        Map<int, std::string> m({{1, "1"}, {2, "2"}});

        auto it1 = m.begin();
        auto it2 = ++m.begin();

        REQUIRE_NE(it1, it2);

        CHECK_EQ(++map::Iterator<int, std::string>(it1), it2);
        CHECK_NE(it1++, it2);
        CHECK_EQ(it1, it2);
    }

    SUBCASE("deref end") {
        Map<int, std::string> m({{1, "1"}});

        auto begin = m.begin();
        auto end = m.end();

        REQUIRE_NOTHROW(*begin);
        CHECK_THROWS_WITH_AS(*end, "iterator is invalid", const IndexError&);
    }

    SUBCASE("stringification") {
        CHECK_EQ(to_string(Map<int, int>({{1, 11}}).begin()), "<map iterator>");
        CHECK_EQ(to_string(Map<int, int>({{1, 11}}).cbegin()), "<const map iterator>");

        CHECK_EQ(fmt("%s", Map<int, int>({{1, 11}}).begin()), "<map iterator>");
        CHECK_EQ(fmt("%s", Map<int, int>({{1, 11}}).cbegin()), "<const map iterator>");
    }
}

TEST_SUITE_END();