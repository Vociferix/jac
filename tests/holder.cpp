#include <catch2/catch_test_macros.hpp>
#include <jac/holder.hpp>

using namespace jac;

TEST_CASE("holder initialization", "[holder]") {
    holder<int> int_value{42};
    holder<int&> ref_value{*int_value};
    holder<int&&> rref_value{*int_value};
    holder<void> void_value{*int_value};
    holder<int[4]> array_value{1, 2, 3, 4};
    holder<int(&)[4]> array_ref_value{*array_value};

    REQUIRE(*int_value == 42);
    REQUIRE(&*ref_value == &*int_value);
    REQUIRE(*rref_value == 42);
    REQUIRE(*void_value == void_v);
    REQUIRE((*array_value)[0] == 1);
    REQUIRE((*array_value)[1] == 2);
    REQUIRE((*array_value)[2] == 3);
    REQUIRE((*array_value)[3] == 4);
    REQUIRE(&(*array_ref_value)[0] == &(*array_value)[0]);
    REQUIRE(&(*array_ref_value)[1] == &(*array_value)[1]);
    REQUIRE(&(*array_ref_value)[2] == &(*array_value)[2]);
    REQUIRE(&(*array_ref_value)[3] == &(*array_value)[3]);
}

TEST_CASE("holder copy", "[holder]") {
    holder<int> int_value{42};
    holder<int&> ref_value{*int_value};
    holder<int&&> rref_value{*int_value};
    holder<void> void_value{*int_value};
    holder<int[4]> array_value{1, 2, 3, 4};
    holder<int(&)[4]> array_ref_value{*array_value};

    holder<int> int_copy = int_value;
    holder<int&> ref_copy = ref_value;
    holder<int&&> rref_copy = rref_value;
    holder<void> void_copy = void_value;
    holder<int[4]> array_copy = array_value;
    holder<int(&)[4]> array_ref_copy = array_ref_value;

    REQUIRE(int_copy == int_value);
    REQUIRE(&*ref_copy == &*ref_value);
    REQUIRE(rref_copy == rref_value);
    REQUIRE(void_copy == void_value);
    REQUIRE((*array_copy)[0] == (*array_value)[0]);
    REQUIRE((*array_copy)[1] == (*array_value)[1]);
    REQUIRE((*array_copy)[2] == (*array_value)[2]);
    REQUIRE((*array_copy)[3] == (*array_value)[3]);
    REQUIRE(&(*array_ref_copy)[0] == &(*array_ref_value)[0]);
    REQUIRE(&(*array_ref_copy)[1] == &(*array_ref_value)[1]);
    REQUIRE(&(*array_ref_copy)[2] == &(*array_ref_value)[2]);
    REQUIRE(&(*array_ref_copy)[3] == &(*array_ref_value)[3]);
}

TEST_CASE("holder swap", "[holder]") {
    holder<int> int1{42};
    holder<int> int2{24};
    swap(int1, int2);
    REQUIRE(*int1 == 24);
    REQUIRE(*int2 == 42);

    holder<int&> ref1{*int1};
    holder<int&> ref2{*int2};
    swap(ref1, ref2);
    REQUIRE(&*ref1 == &*int2);
    REQUIRE(&*ref2 == &*int1);

    holder<int&&> rref1{*int1};
    holder<int&&> rref2{*int2};
    swap(rref1, rref2);
    REQUIRE(*rref1 == *int2);
    REQUIRE(*rref2 == *int1);

    holder<void> void1{*int1};
    holder<void> void2{*int2};
    swap(void1, void2);
    REQUIRE(void1 == void2); // this is probably a pointless check

    holder<int[4]> array1{1, 2, 3, 4};
    holder<int[4]> array2{4, 3, 2, 1};
    swap(array1, array2);
    REQUIRE((*array1)[0] == 4);
    REQUIRE((*array1)[1] == 3);
    REQUIRE((*array1)[2] == 2);
    REQUIRE((*array1)[3] == 1);
    REQUIRE((*array2)[0] == 1);
    REQUIRE((*array2)[1] == 2);
    REQUIRE((*array2)[2] == 3);
    REQUIRE((*array2)[3] == 4);

    holder<int(&)[4]> array_ref1{*array1};
    holder<int(&)[4]> array_ref2{*array2};
    swap(array_ref1, array_ref2);
    REQUIRE(&(*array_ref1)[0] == &(*array2)[0]);
    REQUIRE(&(*array_ref1)[1] == &(*array2)[1]);
    REQUIRE(&(*array_ref1)[2] == &(*array2)[2]);
    REQUIRE(&(*array_ref1)[3] == &(*array2)[3]);
    REQUIRE(&(*array_ref2)[0] == &(*array1)[0]);
    REQUIRE(&(*array_ref2)[1] == &(*array1)[1]);
    REQUIRE(&(*array_ref2)[2] == &(*array1)[2]);
    REQUIRE(&(*array_ref2)[3] == &(*array1)[3]);
}
