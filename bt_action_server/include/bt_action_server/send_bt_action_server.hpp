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

#ifndef SendBTActionServer_H
#define SendBTActionServer_H

#pragma once

#include <bt_executor/bt_executor.h>

#include <behavior_tree_msgs/action/send_behavior_tree.hpp>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

namespace bt_action_server
{

class SendBTActionServer
{
public:
  using ActionType = behavior_tree_msgs::action::SendBehaviorTree;
  using GoalHandleTree = rclcpp_action::ServerGoalHandle<ActionType>;
  using BTExecutorPtr = std::shared_ptr<behavior_tree_executor::BehaviorTreeExecutor>;
  /**
   * @brief Construct a new action server class that exposes the ability to load a tree from a sent
   * xml string once the action is called.
   *
   * @param tree the pointer to the executor.
   * @param node the pointer to the ROS2 node.
   */
  explicit SendBTActionServer(BTExecutorPtr tree, std::shared_ptr<rclcpp::Node> node);
  /**
   * @brief Destructor of the SendBTActionServer.
   *
   */
  ~SendBTActionServer(void) {}

protected:
  rclcpp::Node::SharedPtr node_;

  rclcpp_action::Server<ActionType>::SharedPtr bt_action_server_;

  std::string bt_action_server_name_;
  bool success_{true};

  BTExecutorPtr tree_;

  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const ActionType::Goal> goal);

  rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandleTree> goal_handle);

  void handle_accepted(const std::shared_ptr<GoalHandleTree> goal_handle);

  void execute(const std::shared_ptr<GoalHandleTree> goal_handle);
};
}  // namespace bt_action_server
#endif