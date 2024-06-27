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

#include "bt_action_server/send_bt_action_server.hpp"

#include <functional>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

namespace bt_action_server
{

SendBTActionServer::SendBTActionServer(BTExecutorPtr tree, std::shared_ptr<rclcpp::Node> node)
: node_(node), tree_(tree)
{
  using namespace std::placeholders;

  this->bt_action_server_ = rclcpp_action::create_server<ActionType>(
    node_, "bt_action_server", std::bind(&SendBTActionServer::handle_goal, this, _1, _2),
    std::bind(&SendBTActionServer::handle_cancel, this, _1),
    std::bind(&SendBTActionServer::handle_accepted, this, _1));
}

rclcpp_action::GoalResponse SendBTActionServer::handle_goal(
  const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const ActionType::Goal> goal)
{
  RCLCPP_INFO(node_->get_logger(), "Received goal request");
  (void)uuid;
  return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse SendBTActionServer::handle_cancel(
  const std::shared_ptr<GoalHandleTree> goal_handle)
{
  RCLCPP_INFO(node_->get_logger(), "Received request to cancel goal");
  (void)goal_handle;
  tree_->stop();
  return rclcpp_action::CancelResponse::ACCEPT;
}

void SendBTActionServer::handle_accepted(const std::shared_ptr<GoalHandleTree> goal_handle)
{
  using namespace std::placeholders;
  // this needs to return quickly to avoid blocking the executor, so spin up a new thread
  std::thread{std::bind(&SendBTActionServer::execute, this, _1), goal_handle}.detach();
}

void SendBTActionServer::execute(const std::shared_ptr<GoalHandleTree> goal_handle)
{
  const auto goal = goal_handle->get_goal();
  auto tree_xml = goal->tree.behavior_tree_xml;
  auto goal_tree_name = tree_->extractNameFromXml(tree_xml);
  RCLCPP_INFO_STREAM(node_->get_logger(), "Execution of tree: " << goal_tree_name << " requested!");

  rclcpp::Rate loop_rate(1);
  if (tree_->isTreeInMap(goal_tree_name)) {
    RCLCPP_WARN_STREAM(node_->get_logger(), "Tree already in map, overriding...");
  }

  std::map<std::string, std::string> map{};
  for (auto const & m : goal->tree.blackboard.map) {
    map[m.key] = m.value;
  }

  auto feedback = std::make_shared<ActionType::Feedback>();
  auto result = std::make_shared<ActionType::Result>();

  if (!tree_->loadTree(tree_xml, map)) {
    result->success = false;
    result->message = "Was not able to load tree!";
    goal_handle->abort(result);
    RCLCPP_INFO_STREAM(node_->get_logger(), result->message);
    return;
  }

  if (!tree_->runTree(goal_tree_name)) {
    result->success = false;
    result->message = "Was not able to run tree!";
    goal_handle->abort(result);
    RCLCPP_INFO_STREAM(node_->get_logger(), result->message);
    return;
  }

  success_ = true;
  while (!tree_->isStopped()) {
    if (goal_handle->is_canceling()) {
      goal_handle->canceled(result);
      RCLCPP_INFO(node_->get_logger(), "Goal canceled");
      result->message = "Goal canceled!";
      tree_->stop();
      return;
    }
    goal_handle->publish_feedback(feedback);
    loop_rate.sleep();
  }

  // Check if goal is done
  if (rclcpp::ok()) {
    if (tree_->isLastTreeFailed()) {
      result->success = false;
      result->message = "Behavior Tree failed!";
    } else {
      result->success = true;
      result->message = "Behavior Tree succeeded!";
    }

    goal_handle->succeed(result);
    RCLCPP_INFO_STREAM(node_->get_logger(), result->message);
  }
}

}  // namespace bt_action_server
