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
// Author: Michael Terzer (michael.terzer@fraunhofer.it), Tobit Flatscher

#include "bt_executor/bt_skill_manager.h"

#include <rclcpp/rclcpp.hpp>

#include "behaviortree_ros2/ros_node_params.hpp"

namespace behavior_tree_executor
{
BehaviorTreeSkillManager::BehaviorTreeSkillManager(
  std::shared_ptr<rclcpp::Node> node, BT::BehaviorTreeFactory & factory)
: node_(node), factory_(factory)
{
  skill_plugin_loader_ = std::make_unique<pluginlib::ClassLoader<bt_skill::SkillLibraryBase>>(
    "bt_executor", "bt_skill::SkillLibraryBase");
  return;
}

bool BehaviorTreeSkillManager::loadPlugins(std::vector<std::string> const & plugin_names)
{
  for (auto const & p : plugin_names) {
    this->loadPlugin(p);
  }
  return true;
}

bool BehaviorTreeSkillManager::loadPlugin(std::string const & plugin_name)
{
  if (skill_plugin_loader_->isClassAvailable(plugin_name)) {
    std::shared_ptr<bt_skill::SkillLibraryBase> skill_library =
      skill_plugin_loader_->createSharedInstance(plugin_name);
    BT::RosNodeParams params;
    params.nh = node_;
    skill_library->load(factory_, params);
    RCLCPP_INFO_STREAM(
      node_->get_logger(), "Successfully loaded skill library '" << plugin_name << "'.");
  } else {
    RCLCPP_INFO_STREAM(node_->get_logger(), "Skill library '" << plugin_name << "' not loaded.");
  }
  return true;
}

std::vector<std::string> BehaviorTreeSkillManager::getPlugins(
  std::vector<std::string> const & package_names)
{
  const std::vector<std::string> & available_plugin_names =
    skill_plugin_loader_->getDeclaredClasses();

  RCLCPP_INFO_STREAM(
    node_->get_logger(),
    "Available 'bt_skill::SkillLibraryBase' plugins found in the environment: ");
  for (auto const & plugin : available_plugin_names) {
    RCLCPP_INFO_STREAM(node_->get_logger(), plugin);
  }

  std::vector<std::string> plugin_names{};
  if (package_names.size() > 0) {
    for (auto const & p : package_names) {
      bool is_found{false};
      std::regex const r{"^" + p + "::([A-Za-z_0-9]+)"};
      for (auto const & a : available_plugin_names) {
        std::smatch res{};
        if (std::regex_search(a, res, r)) {
          plugin_names.push_back(a);
          is_found = true;
        }
      }
      if (is_found == false) {
        RCLCPP_WARN_STREAM(
          node_->get_logger(), "Discarding '" << p << "': Plug-in could not be found.");
      }
    }
  } else {
    RCLCPP_WARN_STREAM(
      node_->get_logger(),
      "No package list given as input argument, using all available plug-ins.");
    plugin_names = available_plugin_names;
  }
  return plugin_names;
}

}  // namespace behavior_tree_executor