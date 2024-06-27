// Copyright 2022-2024 Fraunhofer Italia Research

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
//
// Copyright 2022 Fraunhofer Italia Research. All Rights Reserved.
// Author: Tobit Flatscher

#ifndef BEHAVIOR_TREE_MSGS_PARSING_UTILS
#define BEHAVIOR_TREE_MSGS_PARSING_UTILS
#pragma once

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <array>
#include <boost/type_index.hpp>
#include <cassert>
#include <regex>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace behavior_tree_msgs
{

/**\class 
   *   is_always_false
   * \brief
   *   Type trait that is always false
   *   Used to avoid problem with static_assert(false), 
   *   see https://stackoverflow.com/questions/53945490/how-to-assert-that-a-constexpr-if-else-clause-never-happen
   *
   * \tparam Ts
   *   Some data type
  */
template <class... Ts>
struct is_always_false : std::false_type
{
};
// Alias template for convenience
template <typename... Ts>
inline constexpr auto is_always_false_v = is_always_false<Ts...>::value;

/**\fn toVec
   * \brief 
   *   Function for converting a string containing numbers to a vector of type T
   *   The numbers might be given in scientific notation -X.XXXe+YYY where the signs as well as the exponent are optional
   *   The separator does not matter due to the usage of general regular expressions
   *
   * \tparam T
   *   Floating point number
   * \tparam
   *   Used for SFINAE: Only numeric data types
   * \param[in] str
   *   The string to be converted to a vector of numbers
   * \return
   *   A vector containing the numbers contained in the string
  */
template <
  typename T,
  typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>> * = nullptr>
std::vector<T> toVec(std::string const & str) noexcept
{
  std::regex const scientific_numbers_expr{
    "((\\+|-)?[[:digit:]]+)(\\.(([[:digit:]]+)?))?((e|E)((\\+|-)?)[[:digit:]]+)?"};
  std::vector<T> vec{};
  for (auto it = std::sregex_iterator(str.begin(), str.end(), scientific_numbers_expr);
       it != std::sregex_iterator(); ++it) {
    std::string const match{it->str()};
    if constexpr (std::is_same_v<T, float>) {
      vec.push_back(std::stof(match));
    } else if constexpr (std::is_same_v<T, double>) {
      vec.push_back(std::stod(match));
    } else if constexpr (std::is_signed_v<T>) {
      vec.push_back(static_cast<T>(std::stoll(match)));
    } else if constexpr (std::is_unsigned_v<T>) {
      vec.push_back(static_cast<T>(std::stoull(match)));
    } else {
      static_assert(is_always_false_v<T>, "No known conversion to string for type T.");
    }
  }
  return vec;
}

/**\fn toArr
   * \brief
   *   Function for converting a string containing numbers to a fixed-sized array of type T
   *   The numbers might be given in scientific notation -X.XXXe+YYY where the different signs are optional
   *   The separator does not matter due to the usage of general regular expressions
   *
   * \tparam T 
   *   Floating point number
   * \tparam
   *   Used for SFINAE: Only numeric data types
   * \tparam N
   *   The length of the array
   * \param[in] str
   *   The string to be converted to a vector of numbers
   * \return
   *   An array containing the numbers contained in the string
  */
template <
  typename T, std::size_t N,
  typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>> * = nullptr>
std::array<T, N> toArr(std::string const & str)
{
  std::array<T, N> arr{};
  auto const vec{toVec<T>(str)};
  if (vec.size() == N) {
    std::copy_n(vec.begin(), N, arr.begin());
  } else {
    throw std::runtime_error(
      "Wrong number of elements: " + std::to_string(N) + " expected, " +
      std::to_string(vec.size()) + " given.");
  }
  return arr;
}

/**\fn split
   * \brief
   *   Split a string of valid C++ variable names, separated by special characters or spaces into 
   *
   * \param[in] str
   *   The string to be split
   * \return
   *   A vector containing the individual names inside the string
  */
inline std::vector<std::string> split(std::string const & str) noexcept
{
  std::regex const classes_regex{"[A-Za-z_]([A-Za-z_0-9])+"};
  std::vector<std::string> vec{};
  for (auto it = std::sregex_iterator(str.begin(), str.end(), classes_regex);
       it != std::sregex_iterator(); ++it) {
    vec.push_back(it->str());
  }
  return vec;
}

/**\fn getTypeName
   * \brief
   *   Get the name of a data type without its namespace by reversing name mangling
   *   Simply removes the namespace from the data type that Boost TypeIndex returns
   *
   * \return
   *   The type of the class without its namespace
  */
template <typename T>
std::string getTypeName(bool remove_namespace = true)
{
  auto const name{boost::typeindex::type_id<T>().pretty_name()};
  if (!remove_namespace) return name;
  std::regex const r{"[A-Za-z_]([A-Za-z_0-9])+$"};
  auto it = std::sregex_iterator(name.begin(), name.end(), r);
  if (it != std::sregex_iterator()) {
    return it->str();
  } else {
    throw std::invalid_argument("Can't unmangle type name.");
  }
}

/**\fn pop_front
   * \brief
   *   Take the first element from the given YAML::NodeSequence and return the first element as well 
   *   as the remaining elements of the sequence
   *
   * \param[in] in
   *   The YAML::NodeSequence that the operation should be performed on
   * \return
   *   The first element of the given YAML as well as a list of the remaining elements
  */
template <typename T>
[[nodiscard]] std::pair<T, YAML::Node> pop_front(YAML::Node const & in)
{
  YAML::Node out{in};
  if (out.size() < 1) {
    throw std::invalid_argument("Pop front not possible: YAML node returned size <= 1.");
  }
  std::size_t const idx{0};
  auto const first = out[idx].as<T>();
  out.remove(idx);
  return std::make_pair(first, out);
}

/**\fn pop_back
   * \brief
   *   Take the last element from the given YAML::NodeSequence and return the last element as well 
   *   as the remaining elements of the sequence
   *
   * \param[in] in
   *   The YAML::NodeSequence that the operation should be performed on
   * \return
   *   The last element of the given YAML as well as a list of the remaining elements
  */
template <typename T>
[[nodiscard]] std::pair<T, YAML::Node> pop_back(YAML::Node const & in)
{
  YAML::Node out{in};
  if (out.size() < 1) {
    throw std::invalid_argument("Pop back not possible: YAML node returned size <= 1.");
  }
  std::size_t const idx{out.size() - 1};
  auto const last = out[idx].as<T>();
  out.remove(idx);
  return std::make_pair(last, out);
}

}  // namespace behavior_tree_msgs

#endif  // BEHAVIOR_TREE_MSGS_PARSING_UTILS
