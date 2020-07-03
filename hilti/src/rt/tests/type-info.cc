// Copyright (c) 2020 by the Zeek Project. See LICENSE for details.

#include <doctest/doctest.h>

#include <hilti/rt/type-info.h>

using namespace hilti::rt;

TEST_SUITE_BEGIN("TypeInfo");

/* HILTI code to generate the type information used in this test:

module Test {

type X = struct {
    int<32> i;
    string s;
    Y y;
};

type Y = struct {
    bool b;
    real r;
};

*/

// Copied from output of hiltic.
namespace __hlt::type_info {
namespace {
extern const hilti::rt::TypeInfo __ti_Test_X;
extern const hilti::rt::TypeInfo __ti_Test_Y;
} // namespace
} // namespace __hlt::type_info

namespace Test {

// Reduced declaration of the struct types, trusting that ours will match the
// layoput coming out of HILTI ...
struct Y {
    hilti::rt::Bool b;
    double r;
};

struct X {
    hilti::rt::integer::safe<int32_t> i;
    std::string s;
    Y y;
};
} // namespace Test

// Copied from output of hiltic.
namespace __hlt::type_info {
namespace {
const hilti::rt::TypeInfo __ti_Test_X =
    {"Test::X", "Test::X",
     hilti::rt::type_info::Struct(std::vector<hilti::rt::type_info::struct_::Field>(
         {hilti::rt::type_info::struct_::Field{"i", &hilti::rt::type_info::int32, offsetof(Test::X, i)},
          hilti::rt::type_info::struct_::Field{"s", &hilti::rt::type_info::string, offsetof(Test::X, s)},
          hilti::rt::type_info::struct_::Field{"y", &type_info::__ti_Test_Y, offsetof(Test::X, y)}}))};
const hilti::rt::TypeInfo __ti_Test_Y =
    {"Test::Y", "Test::Y",
     hilti::rt::type_info::Struct(std::vector<hilti::rt::type_info::struct_::Field>(
         {hilti::rt::type_info::struct_::Field{"b", &hilti::rt::type_info::bool_, offsetof(Test::Y, b)},
          hilti::rt::type_info::struct_::Field{"r", &hilti::rt::type_info::real, offsetof(Test::Y, r)}}))};
} // namespace
} // namespace __hlt::type_info

TEST_CASE("traverse structs") {
    // Check that we can traverse the structs and get exepcted values.

    auto sx = StrongReference<Test::X>(Test::X{42, "foo", Test::Y{true, 3.14}});
    auto p = type_info::value::Parent(sx);
    auto v = type_info::Value(&*sx, &__hlt::type_info::__ti_Test_X, p);

    auto x = type_info::value::auxType<type_info::Struct>(v).iterate(v);
    auto xi = x.begin();
    auto xf1 = type_info::value::auxType<type_info::SignedInteger<int32_t>>(xi->second).get(xi->second);

    CHECK(xf1 == 42);
    xi++;

    auto xf2 = type_info::value::auxType<type_info::String>(xi->second).get(xi->second);
    CHECK(xf2 == std::string("foo"));
    xi++;

    auto y = type_info::value::auxType<type_info::Struct>(xi->second).iterate(xi->second);
    auto yi = y.begin();

    auto yf1 = type_info::value::auxType<type_info::Bool>(yi->second).get(yi->second);
    CHECK(yf1 == true);
    yi++;

    auto yf2 = type_info::value::auxType<type_info::Real>(yi->second).get(yi->second);
    CHECK(yf2 == 3.14);
    yi++;

    xi++;
    CHECK(yi == y.end());
    CHECK(xi == x.end());
}

TEST_CASE("life-time") {
    // Check that we catch when values become inaccessible because of the
    // associated parent going away.
    Test::Y y{true, 3.14};

    auto x = StrongReference<Test::X>(Test::X{42, "foo", y});
    auto p = type_info::value::Parent(x);
    auto v = type_info::Value(&*x, &__hlt::type_info::__ti_Test_X, p);

    // v is valid
    v.pointer();

    p = type_info::value::Parent();

    // Now invalid.
    CHECK_THROWS_WITH_AS(v.pointer(), "type info value expired", const type_info::InvalidValue&);
}

TEST_SUITE_END();
