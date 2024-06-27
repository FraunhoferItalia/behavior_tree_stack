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

#ifndef BTActionServer_H
#define BTActionServer_H

#pragma once

#include <bt_executor/bt_executor.h>

#include <behavior_tree_msgs/action/tree.hpp>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
//#include "rclcpp_components/register_node_macro.hpp"

namespace bt_action_server
{

class BTActionServer
{
public:
  using TreeAction = behavior_tree_msgs::action::Tree;
  using GoalHandleTree = rclcpp_action::ServerGoalHandle<TreeAction>;
  using BTExecutorPtr = std::shared_ptr<behavior_tree_executor::BehaviorTreeExecutor>;
  /**
   * @brief Construct a new action server class that exposes the ability to control the execution
   * of a behavior tree from an action call.
   *
   * @param tree the pointer to the executor.
   * @param node the pointer to the ROS2 node.
   */
  explicit BTActionServer(BTExecutorPtr tree, std::shared_ptr<rclcpp::Node> node);
  /**
   * @brief Destroy the Subtree Action Server object
   *
   */
  ~BTActionServer(void) {}

protected:
  rclcpp::Node::SharedPtr node_;

  behavior_tree_msgs::action::Tree_Feedback feedback_;
  behavior_tree_msgs::action::Tree_Result result_;

  rclcpp_action::Server<TreeAction>::SharedPtr bt_action_server_;

  std::string bt_action_server_name_;
  bool success_{true};

  BTExecutorPtr tree_;

  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const TreeAction::Goal> goal);

  rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandleTree> goal_handle);

  void handle_accepted(const std::shared_ptr<GoalHandleTree> goal_handle);

  void execute(const std::shared_ptr<GoalHandleTree> goal_handle);
};
}  // namespace bt_action_server
#endif