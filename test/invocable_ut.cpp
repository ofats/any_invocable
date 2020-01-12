#include <array>
#include <numeric>

#include "catch2/catch.hpp"
#include "invocable.h"

namespace {

int foo() { return 5; }

struct widget {
  int boo() { return foo(); }
};

struct statefull_widget {
  statefull_widget(int val) : value(val) {}
  statefull_widget(std::initializer_list<int> il)
      : value(std::accumulate(il.begin(), il.end(), 0)) {}
  int operator()() { return value; }
  int value = 0;
};

template <class R, class... ArgTypes>
void require_empty(const ofats::any_invocable<R(ArgTypes...)>& f) {
  REQUIRE(!f);
  REQUIRE(f == nullptr);
  REQUIRE(nullptr == f);
}

template <class R, class... ArgTypes>
void require_non_empty(const ofats::any_invocable<R(ArgTypes...)>& f) {
  REQUIRE(f);
  REQUIRE(f != nullptr);
  REQUIRE(nullptr != f);
}

}  // namespace

TEST_CASE("Construction", "[any_invokable]") {
  using inv_type = ofats::any_invocable<int()>;

  SECTION("Default construction") {
    auto f = inv_type{};
    require_empty(f);
  }

  SECTION("Move construction") {
    auto f = inv_type{foo};
    require_non_empty(f);
    REQUIRE(f() == 5);
    auto f2 = inv_type{std::move(f)};
    require_empty(f);
    require_non_empty(f2);
    REQUIRE(f2() == 5);
  }

  SECTION("Construction from nullptr") {
    auto f = inv_type{nullptr};
    require_empty(f);
  }

  SECTION("Construction from function") {
    auto f = inv_type{foo};
    require_non_empty(f);
    REQUIRE(f() == 5);
  }

  SECTION("Construction from lambda expression") {
    auto f = inv_type{[] { return foo(); }};
    require_non_empty(f);
    REQUIRE(f() == foo());
  }

  SECTION("Construction from member function pointer") {
    auto f = ofats::any_invocable<int(widget*)>{&widget::boo};
    widget w;
    require_non_empty(f);
    REQUIRE(f(&w) == foo());
  }

  SECTION("Construction from large object") {
    auto arr = std::array<int, 32>{};
    arr.fill(foo());
    auto f = inv_type{
        [arr] { return std::accumulate(arr.cbegin(), arr.cend(), 0); }};
    require_non_empty(f);
    REQUIRE(f() == std::accumulate(arr.cbegin(), arr.cend(), 0));
  }

  SECTION("Construction using in_place_type") {
    auto f = inv_type{std::in_place_type<statefull_widget>, foo()};
    require_non_empty(f);
    REQUIRE(f() == foo());
  }

  SECTION("Construction using in_place_type and initializer_list") {
    auto f =
        inv_type{std::in_place_type<statefull_widget>, {foo(), foo(), foo()}};
    require_non_empty(f);
    REQUIRE(f() == foo() * 3);
  }

  SECTION("Construction from move-only callable") {
    auto f = inv_type{[p = std::make_unique<int>(foo())] { return *p; }};
    require_non_empty(f);
    REQUIRE(f() == foo());
  }
}

TEST_CASE("Assignment", "[any_invocable]") {
  ofats::any_invocable<int()> f;
  require_empty(f);

  SECTION("Assignment of default constructed") {
    f = decltype(f){};
    require_empty(f);
  }

  SECTION("Move assignment") {
    auto f2 = decltype(f){foo};
    require_non_empty(f2);
    REQUIRE(f2() == foo());
    f = std::move(f2);
    require_empty(f2);
    require_non_empty(f);
    REQUIRE(f() == foo());
  }

  SECTION("Assignment of nullptr") {
    f = nullptr;
    require_empty(f);
  }

  SECTION("Assignment of function") {
    f = foo;
    require_non_empty(f);
    REQUIRE(f() == 5);
  }

  SECTION("Assignment of lambda expression") {
    f = [] { return foo(); };
    require_non_empty(f);
    REQUIRE(f() == foo());
  }

  SECTION("Assignment of member function pointer") {
    ofats::any_invocable<int(widget*)> f;
    require_empty(f);
    f = &widget::boo;
    require_non_empty(f);
    widget w;
    REQUIRE(f(&w) == foo());
  }

  SECTION("Assignment of reference wrapper") {
    auto lmb = [] { return foo(); };
    f = std::ref(lmb);
    require_non_empty(f);
    REQUIRE(f() == lmb());
  }

  SECTION("Assignment of move-only callable") {
    f = [p = std::make_unique<int>(foo())] { return *p; };
    require_non_empty(f);
    REQUIRE(f() == foo());
  }
}

TEST_CASE("Swap and comparisons", "[any_invocable]") {
  using inv_type = ofats::any_invocable<int()>;
  auto func_inv = inv_type{foo};
  auto lmb_inv = inv_type{[] { return foo(); }};
  auto empty_inv1 = inv_type{};
  auto empty_inv2 = inv_type{};

  SECTION("Swapping non-empty") {
    require_non_empty(func_inv);
    require_non_empty(lmb_inv);
    REQUIRE(func_inv() == lmb_inv());
    func_inv.swap(lmb_inv);

    require_non_empty(func_inv);
    require_non_empty(lmb_inv);
    REQUIRE(func_inv() == lmb_inv());
    lmb_inv.swap(func_inv);

    require_non_empty(func_inv);
    require_non_empty(lmb_inv);
    REQUIRE(func_inv() == lmb_inv());
  }

  SECTION("Swapping empty and non-empty") {
    require_non_empty(func_inv);
    REQUIRE(func_inv() == 5);
    require_empty(empty_inv1);

    func_inv.swap(empty_inv1);
    require_non_empty(empty_inv1);
    REQUIRE(empty_inv1() == 5);
    require_empty(func_inv);

    require_empty(empty_inv2);
    empty_inv2.swap(empty_inv1);
    require_non_empty(empty_inv2);
    REQUIRE(empty_inv2() == 5);
    require_empty(empty_inv1);
  }

  SECTION("Swapping empty") {
    require_empty(empty_inv1);
    require_empty(empty_inv2);

    empty_inv1.swap(empty_inv2);
    require_empty(empty_inv1);
    require_empty(empty_inv2);

    empty_inv2.swap(empty_inv1);
    require_empty(empty_inv1);
    require_empty(empty_inv2);
  }
}

// TODO(ofats): small vs. large tests, number of move,ctor,dtor test,...
