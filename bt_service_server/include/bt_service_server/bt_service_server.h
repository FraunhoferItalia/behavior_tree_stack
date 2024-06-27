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

#ifndef BTServiceServer_H
#define BTServiceServer_H

#pragma once

#include <rclcpp/rclcpp.hpp>

#include "behavior_tree_msgs/srv/load_behavior_tree.hpp"
#include "behavior_tree_msgs/srv/string.hpp"
#include "bt_executor/bt_executor.h"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/trigger.hpp"

namespace bt_service_server
{
class BTServiceServer
{
public:
  /**
   * @brief Construct a new service server class that exposes service servers to control the execution
   * of a behavior tree.
   *
   * @param tree the pointer to the executor.
   * @param node the pointer to the ROS2 node.
   */
  BTServiceServer(behavior_tree_executor::Ptr tree, std::shared_ptr<rclcpp::Node> node);
  /**
   * @brief Destroy the service servers.
   *
   */
  ~BTServiceServer() {}

protected:
  /**
   * @brief plays the default behavior tree (indicated in the yaml files)
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool play_default_clbk(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
    std::shared_ptr<std_srvs::srv::Trigger::Response> resp);
  /**
   * @brief play a specific behavior tree from the trees in the list after launch.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool play_specific_clbk(
    const std::shared_ptr<behavior_tree_msgs::srv::String::Request> req,
    std::shared_ptr<behavior_tree_msgs::srv::String::Response> resp);
  /**
   * @brief load a behavior tree and run it.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool load_and_play_clbk(
    const std::shared_ptr<behavior_tree_msgs::srv::LoadBehaviorTree_Request> req,
    const std::shared_ptr<behavior_tree_msgs::srv::LoadBehaviorTree_Response> resp);
  /**
   * @brief pause a behavior tree.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool pause_clbk(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
    std::shared_ptr<std_srvs::srv::Trigger::Response> resp);
  /**
   * @brief resume a paused behavior tree.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool resume_clbk(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
    std::shared_ptr<std_srvs::srv::Trigger::Response> resp);
  /**
   * @brief stop a running behavior tree.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool stop_clbk(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
    std::shared_ptr<std_srvs::srv::Trigger::Response> resp);
  /**
   * @brief manual tick a behavior tree.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool manual_tick_clbk(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
    std::shared_ptr<std_srvs::srv::Trigger::Response> resp);

  void loop_timer_clbk();

  behavior_tree_executor::Ptr tree_;

  // ROS2 node
  std::shared_ptr<rclcpp::Node> node_;

  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr play_default_srv_;
  rclcpp::Service<behavior_tree_msgs::srv::String>::SharedPtr play_specific_srv_;
  rclcpp::Service<behavior_tree_msgs::srv::LoadBehaviorTree>::SharedPtr load_and_run_srv_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr pause_srv_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr resume_srv_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr stop_srv_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr manual_tick_srv_;

  std::string bt_service_server_name_;
  std::string play_default_tree_service_name_;
  std::string load_and_run_service_name_;
  std::string play_specific_tree_service_name_;
  std::string pause_service_name_;
  std::string resume_service_name_;
  std::string stop_service_name_;
  std::string manual_tick_service_name_;

  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr
    bt_state_publisher_;             ///< the  publisher of behavior tree state.
  std::string bt_state_topic_name_;  ///< the  topic name of the behavior tree state topic.

  rclcpp::TimerBase::SharedPtr timer_;  ///< timer for the bt state topic
  std::chrono::duration<double, std::milli>
    loop_timer_duration_;  ///< duration for the bt state topic
};
}  // namespace bt_service_server
#endif