// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file defines type traits related utilities.

#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

// #include "llvm/ADT/STLExtras.h"

namespace model_parser {

// Utility template for tag dispatching.
template <typename T>
struct TypeTag {};

// This is the equivalent of std::void_t in C++17.
template <typename... Ts>
struct make_void {
  typedef void type;
};
template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

// The same as std::disjunction in C++17.
template <class...>
struct disjunction : std::false_type {};
template <class B1>
struct disjunction<B1> : B1 {};
template <class B1, class... Bn>
struct disjunction<B1, Bn...>
    : std::conditional_t<bool(B1::value), B1, disjunction<Bn...>> {};

// Check whether T may be a base class.
// template <typename T>
// using MaybeBase =
//     llvm::conjunction<std::is_class<T>, llvm::negation<std::is_final<T>>>;

// Find the index of a type in a tuple.
//
// Example:
// using Tuple = std::tuple<int, float, double>;
// static_assert(TupleIndexOf<int, Tuple>::value == 0);
// static_assert(TupleIndexOf<double, Tuple>::value == 2);
template <class T, class Tuple>
struct TupleIndexOf;

template <class T, class... Types>
struct TupleIndexOf<T, std::tuple<T, Types...>>
    : std::integral_constant<size_t, 0> {};

template <class T, class U, class... Types>
struct TupleIndexOf<T, std::tuple<U, Types...>>
    : std::integral_constant<size_t,
                             1 + TupleIndexOf<T, std::tuple<Types...>>::value> {
};

template <typename T, typename Tuple>
struct TupleHasType;

template <typename T, typename... Us>
struct TupleHasType<T, std::tuple<Us...>>
    : disjunction<std::is_same<T, Us>...> {};

// The detector pattern in C++ that can be used for checking whether a type has
// a specific property, e.g. whether an internal type is present or whether a
// particular operation is valid.
//
// Sample usage:
//
// struct Foo {
//   using difference_type = int;
//   int get();
// };
// struct Bar {};
//
// // Check whether a type T has an internal difference_type.
// template<class T>
// using diff_t = typename T::difference_type;
//
// static_assert(is_detected_v<diff_t, Foo>, "Foo has difference_type");
// static_assert(!is_detected_v<diff_t, Bar>, "Bar has no difference_type");
//
// // Check whether a type T has a get() member function.
// template<class T>
// using has_get_t = decltype(std::declval<T>().get());
//
// static_assert(is_detected_v<has_get_t, Foo>, "Foo has get()");
// static_assert(!is_detected_v<has_get_t, Bar>, "Bar has no get()");
//
// See https://en.cppreference.com/w/cpp/experimental/is_detected for details.

namespace internal {

// nonesuch is a class type used to indicate detection failure.
struct nonesuch {
  ~nonesuch() = delete;
  nonesuch(nonesuch const&) = delete;
  void operator=(nonesuch const&) = delete;
};

template <class Default,
          class AlwaysVoid,
          template <class...>
          class Op,
          class... Args>
struct detector : std::false_type {
  using value_t = std::false_type;
  using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
  using value_t = std::true_type;
  using type = Op<Args...>;
};

}  // namespace internal

template <template <class...> class Op, class... Args>
using is_detected =
    typename internal::detector<internal::nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
using detected_t =
    typename internal::detector<internal::nonesuch, void, Op, Args...>::type;

template <class Default, template <class...> class Op, class... Args>
using detected_or = internal::detector<Default, void, Op, Args...>;

template <template <class...> class Op, class... Args>
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

}  // namespace model_parser
