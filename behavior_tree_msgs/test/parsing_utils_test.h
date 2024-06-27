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

#ifndef behavior_tree_msgs_PARSING_UTILS_TEST
#define behavior_tree_msgs_PARSING_UTILS_TEST
#pragma once

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include "behavior_tree_msgs/parsing_utils.h"

namespace behavior_tree_msgs
{

template <typename T>
struct VectorParsingTest : public ::testing::Test
{
};

using ArithmeticDataTypes = ::testing::Types<
  std::int8_t, std::int16_t, std::int32_t, std::int64_t, std::uint8_t, std::uint16_t, std::uint32_t,
  std::uint64_t, float, double>;

TYPED_TEST_SUITE(VectorParsingTest, ArithmeticDataTypes);

TYPED_TEST(VectorParsingTest, parseFloatNumbers)
{
  if constexpr (std::is_floating_point_v<TypeParam>) {
    std::string const input_str{"1.3, 2, +3, -4.2, 1E-4"};
    std::vector<TypeParam> const expected_result{
      static_cast<TypeParam>(1.3), static_cast<TypeParam>(2), static_cast<TypeParam>(3),
      static_cast<TypeParam>(-4.2), static_cast<TypeParam>(1e-4)};

    auto const result = behavior_tree_msgs::toVec<TypeParam>(input_str);

    ASSERT_EQ(result.size(), expected_result.size());

    for (std::size_t i = 0; i < expected_result.size(); ++i) {
      EXPECT_DOUBLE_EQ(result.at(i), expected_result.at(i));
    }
  } else {
    GTEST_SKIP() << "Test only implemented for floating point numbers!";
  }
}

TYPED_TEST(VectorParsingTest, parseIntNumbers)
{
  std::string const input_str{"1, 2, +3, 4, 100"};
  std::vector<TypeParam> const expected_result{
    static_cast<TypeParam>(1), static_cast<TypeParam>(2), static_cast<TypeParam>(3),
    static_cast<TypeParam>(4), static_cast<TypeParam>(100)};

  auto const result = behavior_tree_msgs::toVec<TypeParam>(input_str);

  ASSERT_EQ(result.size(), expected_result.size());

  for (std::size_t i = 0; i < expected_result.size(); ++i) {
    EXPECT_DOUBLE_EQ(result.at(i), expected_result.at(i));
  }
}

}  // namespace behavior_tree_msgs

#endif  // behavior_tree_msgs_PARSING_UTILS_TEST
