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
// Author: Michael Terzer (michael.terzer@fraunhofer.it)

#ifndef BEHAVIORTREE_DESCRIPTOR
#define BEHAVIORTREE_DESCRIPTOR
#pragma once

#include <behaviortree_cpp/basic_types.h>
#include <behaviortree_cpp/blackboard.h>

#include <behavior_tree_msgs/msg/behavior_tree_descriptor.hpp>
#include <behavior_tree_msgs/msg/key_value_map.hpp>
#include <behavior_tree_msgs/msg/key_value_pair.hpp>
#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace behavior_tree_executor
{

/**\class Descriptor
   * \brief Class containing the name of the behavior tree to be executed as well as its blackboard
  */
class Descriptor
{
  // Underlying map type (key, value)
  using MapType = std::map<std::string, std::string>;

public:
  /**\fn        Descriptor
       * \brief     Constructor from behavior tree name and existing blackboard
       *
       * \param[in] name         Name of the behavior tree
       * \param[in] blackboard   Existing blackboard
      */
  Descriptor(std::string const & name, BT::Blackboard::Ptr & blackboard);
  /**\fn        Descriptor
       * \brief     Constructor from behavior tree name and map of parameters for blackboard
       *
       * \param[in] name   Name of the behavior tree
       * \param[in] map    Map of parameters for behavior tree blackboard
      */
  Descriptor(std::string const & name, MapType const & map) noexcept;

  /**\fn        Descriptor
       * \brief     Constructor from corresponding behavior tree descriptor message
       *
       * \param[in] msg   Behavior tree descriptor message
      */
  Descriptor(behavior_tree_msgs::msg::BehaviorTreeDescriptor const & msg) noexcept;

  /**\fn        convertToMap
       * \brief     Function for converting a behavior tree descriptor message to a map
       *
       * \param[in] msg   Behavior tree descriptor message
       * \return    The behavior tree descriptor message converted to a map
      */
  static MapType convertToMap(behavior_tree_msgs::msg::BehaviorTreeDescriptor const & msg) noexcept;

  /**\fn        convertToKeyValueVec
       * \brief     Function for converting a map to a vector of key-value messages
       *
       * \param[in] map   Map of blackboard parameters
       * \return    The blackboard parameters as a vector of key-value pairs
      */
  static std::vector<behavior_tree_msgs::msg::KeyValuePair> convertToKeyValueVec(
    MapType const & map) noexcept;

  /**\fn        createBlackboard
       * \brief     Function for creating a blackboard from a map of blackboard parameters
       * \warning   A template specialization might be required: https://www.behaviortree.dev/tutorial_03_generic_ports/
       *
       * \param[in] map                 Map of blackboard parameters
       * \param[in] parent_blackboard   The parent blackboard to extend
       * \return    A blackboard containing the supplied parameters
      */
  static BT::Blackboard::Ptr createBlackboard(
    MapType const & map, BT::Blackboard::Ptr const parent_blackboard = nullptr);

  /**\fn        toMap
       * \brief     Export the parameters of a blackboard to a parameter map
       *
       * \param[in] blackboard   The blackboard where the parameters should be extracted from
       * \return    The parameters of the blackboard as a map
      */
  static MapType toMap(BT::Blackboard::Ptr const blackboard);

  /**\fn        toMsg
       * \brief     Export the parameters to a corresponding descriptor message
       *
       * \param[in] name         The name of the behavior tree
       * \param[in] blackboard   The blackboard where the parameters should be extracted from
       * \return    The parameters of the blackboard as a descriptor message
      */
  static behavior_tree_msgs::msg::BehaviorTreeDescriptor toMsg(
    std::string const & name, BT::Blackboard::Ptr const blackboard);

  /**\fn     toMap
       * \brief  Export the parameters of a blackboard saved inside the class to a parameter map
       *
       * \return The parameters of the blackboard saved inside the class as a map
      */
  MapType toMap() const;

  /**\fn     toMsg
       * \brief  Export the parameters saved inside this class to a corresponding descriptor message
       *
       * \return The parameters of the blackboard as a descriptor message
      */
  behavior_tree_msgs::msg::BehaviorTreeDescriptor toMsg() const;

  /**\fn     getName
       * \brief  Getter for the name of the behavior tree wrapped inside the class
       *
       * \return The name of the behavior tree
      */
  std::string getName() const noexcept;

  /**\fn     getBlackboard
       * \brief  Getter for the blackboard wrapped inside the class
       *
       * \return The blackboard wrapped inside the class
      */
  BT::Blackboard::Ptr getBlackboard() const noexcept;

  /**\fn     operator<<
       * \brief  Outstream operator for descriptor class
       *
       * \param[in] os   The output stream that should be written to
       * \param[in] d    The descriptor that should be written to the output stream
       * \return The output stream containing information about the descriptor
      */
  friend std::ostream & operator<<(std::ostream & os, Descriptor const & d) noexcept;

protected:
  std::string name_;
  BT::Blackboard::Ptr blackboard_;
};

}  // namespace behavior_tree_executor

#endif  // BEHAVIORTREE_DESCRIPTOR
