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

#ifndef BehaviorTreeSkillManager_H
#define BehaviorTreeSkillManager_H

#pragma once

#include <rclcpp/rclcpp.hpp>

#include "behaviortree_cpp/bt_factory.h"
#include "bt_executor/skill_library_base.h"
#include "pluginlib/class_loader.hpp"

namespace behavior_tree_executor
{
/**
 * @brief This class manages the skill library. The skills are plugins that can be handled through 
 * the functions of this class. 
 * 
 */
class BehaviorTreeSkillManager
{
public:
  BehaviorTreeSkillManager(std::shared_ptr<rclcpp::Node> node, BT::BehaviorTreeFactory & factory);
  /**
   * @brief loops through the list of plugin-names and calls loadPlugin() respectively.
   * 
   * @param plugin_names the list of plugins to be loaded.
   * @return true if successful,
   * @return false otherwise.
   */
  bool loadPlugins(std::vector<std::string> const & plugin_names);
  /**
   * @brief loades the plugin with plugin_name.
   * 
   * @param plugin_name the plugin to be loaded.
   * @return true if successful,
   * @return false otherwise.
   */
  bool loadPlugin(std::string const & plugin_name);
  /**
  * @brief Get a list of all requested plug-ins. If no plug-ins are provided it defaults to all 
  * available ones.
  * 
  * @param package_names Names of the packages to be loaded.
  * @return std::vector<std::string> The class names of the reqested plug-ins.
  */
  std::vector<std::string> getPlugins(std::vector<std::string> const & package_names);

private:
  std::unique_ptr<pluginlib::ClassLoader<bt_skill::SkillLibraryBase>>
    skill_plugin_loader_;               /// the plugin class loader.
  BT::BehaviorTreeFactory & factory_;   /// the behavior tree factory reference.
  std::shared_ptr<rclcpp::Node> node_;  /// a shared pointer to the ROS2 node.
};
}  // namespace behavior_tree_executor
#endif  //BehaviorTreeSkillManager_H