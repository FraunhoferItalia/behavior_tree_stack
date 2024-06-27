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

#ifndef BEHAVIORTREE_SKILL_LIBRARY
#define BEHAVIORTREE_SKILL_LIBRARY
#pragma once

#include <string>

#include "behavior_tree_msgs/parsing_utils.h"
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_ros2/ros_node_params.hpp"
#include "rclcpp/rclcpp.hpp"

namespace bt_skill
{

/**\class SkillLibrary
   * \brief Base class for registering skills with the behavior tree manager 
  */
class SkillLibraryBase
{
public:
  /**\fn    SkillLibrary
       * \brief Constructor for a skill library
      */
  SkillLibraryBase() = default;
  SkillLibraryBase(SkillLibraryBase const &) = default;
  SkillLibraryBase & operator=(SkillLibraryBase const &) = default;
  SkillLibraryBase(SkillLibraryBase &&) = default;
  SkillLibraryBase & operator=(SkillLibraryBase &&) = default;
  virtual ~SkillLibraryBase() { return; }

  /**\fn registerNodeType
       * \brief
       *   Register a node of a given type with its default name (corresponding to the class name without namespace)
       * 
       * \param[in] factory
       *   Behavior tree factory where the individual skills should be added to
       * \param[in] registration_id
       *   Name under which the action will be registered
      */
  template <typename NodeT>
  static void registerNodeType(
    BT::BehaviorTreeFactory & factory, BT::RosNodeParams & params,
    std::string const & registration_id = behavior_tree_msgs::getTypeName<NodeT>())
  {
    factory.registerNodeType<NodeT>(registration_id, params);
    return;
  }

  /**\fn load
       * \brief
       *   Load the desired skills to the given behavior tree factory.
       *   The manager will call this function when loading this skill library.
       * 
       * \param[in] factory
       *   Behavior tree factory where the individual skills should be added to
      */
  virtual void load(BT::BehaviorTreeFactory & factory, BT::RosNodeParams & params) const = 0;

  /**\fn load
       * \brief
       *   Load the desired skills to the given behavior tree factory.
       *   The manager will call this function when loading this skill library.
       * 
       * \param[in] factory
       *   Behavior tree factory where the individual skills should be added to
       * \param[in] nh
       *   Node handle used for registering actions that require a node handle
      */
  //virtual void load(BT::BehaviorTreeFactory& factory, std::shared_ptr<rclcpp::Node> node) const;
};

}  // namespace bt_skill

#endif  // BEHAVIORTREE_SKILL_LIBRARY
