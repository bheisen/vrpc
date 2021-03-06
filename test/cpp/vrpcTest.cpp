#include <catch.hpp>
#include <tuple>
#include <set>
#include "../vrpc/json.hpp"
#include "../vrpc/vrpc.hpp"

using namespace vrpc;

TEST_CASE("Test json packing and unpacking", "[packing]") {

  SECTION("Packing empty json") {
    json j;
    vrpc::pack(j);
    REQUIRE(j.empty());
  }

  SECTION("Packing json with simple values") {
    json j;
    vrpc::pack(j, 5, "Hello", false, std::vector<int>({1, 2, 3}));
    REQUIRE(j["_1"] == 5);
    REQUIRE(j["_2"] == "Hello");
    REQUIRE(j["_3"] == false);
    REQUIRE(j["_4"] == std::vector<int>({1, 2, 3}));
  }

  SECTION("Packing json with nested values") {
    json j;
    json inner{{"key1", "innerValue"}, {"key2", 2}};
    vrpc::pack(j, "test", inner);
    REQUIRE(j["_1"] == "test");
    REQUIRE(j["_2"]["key1"] == "innerValue");
    REQUIRE(j["_2"]["key2"] == 2);
  }

  SECTION("Check signature generation for empty json") {
    std::string s(vrpc::get_signature());
    REQUIRE(s.empty());
  }

  SECTION("Check signature generation using simple values") {
    std::string s(vrpc::get_signature<int, std::string, double, std::vector<char>>());
    REQUIRE(s == "-numberstringnumberarray");
  }
}

TEST_CASE("Test functionality of generic holder", "[Value]") {

  SECTION("Check retrieval with get") {
    // Store an integer to Value
    Value v(1);
    // by value
    int iv = v.get<int>();
    REQUIRE(iv == 1);
    // by const reference
    const int& ir = v.get<int>();
    REQUIRE(ir == 1);
    // by std::shared_ptr
    std::shared_ptr<int> ip = v.get<std::shared_ptr<int> >();
    REQUIRE(*ip == 1);
    auto ia = v.get<int>();
    REQUIRE(ia == 1);
  }
}
