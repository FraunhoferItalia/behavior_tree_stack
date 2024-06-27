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

#include "bt_service_server/bt_service_server.h"

#include "std_msgs/msg/string.h"

using namespace std::chrono_literals;

namespace bt_service_server
{
BTServiceServer::BTServiceServer(
  behavior_tree_executor::Ptr tree, std::shared_ptr<rclcpp::Node> node)
: tree_(tree), node_(node)
{
  node_->declare_parameter<std::string>("bt_service_server_name", "bt_service_server");
  node_->declare_parameter<std::string>("play_default_tree_service_name", "play_default_tree");
  node_->declare_parameter<std::string>("load_and_run_service_name", "load_and_run");
  node_->declare_parameter<std::string>("play_specific_tree_service_name", "play_specific");
  node_->declare_parameter<std::string>("pause_service_name", "pause");
  node_->declare_parameter<std::string>("resume_service_name", "resume");
  node_->declare_parameter<std::string>("stop_service_name", "stop");
  node_->declare_parameter<std::string>("manual_tick_service_name", "manual_tick");
  node_->declare_parameter<std::string>("bt_state_topic_name", "bt_state");
  node_->declare_parameter<double>("update_dt_sec", 0.1);

  rclcpp::Parameter bt_service_server_name_parm_ = node_->get_parameter("bt_service_server_name");
  bt_service_server_name_ = bt_service_server_name_parm_.as_string();

  rclcpp::Parameter play_default_tree_service_name_param =
    node_->get_parameter("play_default_tree_service_name");
  play_default_tree_service_name_ = play_default_tree_service_name_param.as_string();

  rclcpp::Parameter play_specific_tree_service_name_param =
    node_->get_parameter("play_specific_tree_service_name");
  play_specific_tree_service_name_ = play_specific_tree_service_name_param.as_string();

  rclcpp::Parameter load_and_run_service_name_param =
    node_->get_parameter("load_and_run_service_name");
  load_and_run_service_name_ = load_and_run_service_name_param.as_string();

  rclcpp::Parameter pause_service_name_param = node_->get_parameter("pause_service_name");
  pause_service_name_ = pause_service_name_param.as_string();

  rclcpp::Parameter resume_service_name_param = node_->get_parameter("resume_service_name");
  resume_service_name_ = resume_service_name_param.as_string();

  rclcpp::Parameter stop_service_name_param = node_->get_parameter("stop_service_name");
  stop_service_name_ = stop_service_name_param.as_string();

  rclcpp::Parameter manual_tick_service_name_param =
    node_->get_parameter("manual_tick_service_name");
  manual_tick_service_name_ = manual_tick_service_name_param.as_string();

  rclcpp::Parameter bt_state_topic_name_param = node_->get_parameter("bt_state_topic_name");
  bt_state_topic_name_ = bt_state_topic_name_param.as_string();

  rclcpp::Parameter update_dt_sec = node_->get_parameter("update_dt_sec");
  loop_timer_duration_ = std::chrono::duration<double>(update_dt_sec.as_double());
  RCLCPP_INFO_STREAM(node->get_logger(), loop_timer_duration_.count());

  // init publisher
  bt_state_publisher_ = node_->create_publisher<std_msgs::msg::String>(bt_state_topic_name_, 10);

  // init services
  play_default_srv_ = node_->create_service<std_srvs::srv::Trigger>(
    play_default_tree_service_name_,
    std::bind(
      &BTServiceServer::play_default_clbk, this, std::placeholders::_1, std::placeholders::_2));

  play_specific_srv_ = node_->create_service<behavior_tree_msgs::srv::String>(
    play_specific_tree_service_name_,
    std::bind(
      &BTServiceServer::play_specific_clbk, this, std::placeholders::_1, std::placeholders::_2));

  load_and_run_srv_ = node_->create_service<behavior_tree_msgs::srv::LoadBehaviorTree>(
    load_and_run_service_name_,
    std::bind(
      &BTServiceServer::load_and_play_clbk, this, std::placeholders::_1, std::placeholders::_2));

  pause_srv_ = node_->create_service<std_srvs::srv::Trigger>(
    pause_service_name_,
    std::bind(&BTServiceServer::pause_clbk, this, std::placeholders::_1, std::placeholders::_2));

  stop_srv_ = node_->create_service<std_srvs::srv::Trigger>(
    stop_service_name_,
    std::bind(&BTServiceServer::stop_clbk, this, std::placeholders::_1, std::placeholders::_2));

  resume_srv_ = node_->create_service<std_srvs::srv::Trigger>(
    resume_service_name_,
    std::bind(&BTServiceServer::resume_clbk, this, std::placeholders::_1, std::placeholders::_2));

  manual_tick_srv_ = node_->create_service<std_srvs::srv::Trigger>(
    manual_tick_service_name_,
    std::bind(
      &BTServiceServer::manual_tick_clbk, this, std::placeholders::_1, std::placeholders::_2));

  // init timer
  timer_ = node_->create_wall_timer(
    loop_timer_duration_, std::bind(&BTServiceServer::loop_timer_clbk, this));

  RCLCPP_INFO_STREAM(node_->get_logger(), "BT service server started successfully...");
}

bool BTServiceServer::play_default_clbk(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
  std::shared_ptr<std_srvs::srv::Trigger::Response> resp)
{
  if (!tree_->isStopped()) {
    std::string msg{"Cannot play when tree is not stopped!"};
    RCLCPP_WARN_STREAM(node_->get_logger(), msg);
    resp->message = msg;
    resp->success = false;
    return true;
  }

  tree_->runDefaultTree();

  resp->success = true;
  return true;
}

bool BTServiceServer::play_specific_clbk(
  const std::shared_ptr<behavior_tree_msgs::srv::String::Request> req,
  std::shared_ptr<behavior_tree_msgs::srv::String::Response> resp)
{
  if (!tree_->isStopped()) {
    std::string msg{"Cannot play when tree is not stopped!"};
    RCLCPP_WARN_STREAM(node_->get_logger(), msg);
    resp->message = msg;
    resp->success = false;
    return true;
  }
  tree_->runTree(req->str);
  resp->success = true;
  return true;
}

bool BTServiceServer::load_and_play_clbk(
  const std::shared_ptr<behavior_tree_msgs::srv::LoadBehaviorTree_Request> req,
  const std::shared_ptr<behavior_tree_msgs::srv::LoadBehaviorTree_Response> resp)
{
  behavior_tree_executor::Descriptor const descriptor{req->tree};

  std::string tree_name{};

  tree_name = tree_->extractNameFromDescriptor(descriptor);
  if (tree_name.empty()) {
    return false;
  }

  RCLCPP_INFO_STREAM(node_->get_logger(), "Recieving load and play request of tree: " << tree_name);

  if (tree_->isTreeInMap(tree_name)) {
    RCLCPP_WARN_STREAM(node_->get_logger(), "Tree already in map, overriding...");
  }

  if (!tree_->loadTree(descriptor)) {
    RCLCPP_WARN_STREAM(node_->get_logger(), "Was not able to load tree from descriptor...");
  }

  if (!tree_->isStopped()) {
    RCLCPP_WARN_STREAM(node_->get_logger(), "Cannot play when tree is not stopped!");
    resp->success = false;
    return true;
  }
  try {
    tree_->runTree(tree_name);
  } catch (const std::exception & e) {
    RCLCPP_ERROR_STREAM(node_->get_logger(), e.what());
    return false;
  }

  resp->success = true;
  return true;
}

bool BTServiceServer::pause_clbk(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
  std::shared_ptr<std_srvs::srv::Trigger::Response> resp)
{
  if (!tree_->isRunning()) {
    std::string msg{"Cannot pause when executor is not running!"};
    RCLCPP_WARN_STREAM(node_->get_logger(), msg);
    resp->message = msg;
    resp->success = false;
    return true;
  }
  RCLCPP_WARN_STREAM(node_->get_logger(), "Pause tree...");
  if (!tree_->pause()) {
    std::string msg{"Cannot pause when tree is not running!"};
    RCLCPP_WARN_STREAM(node_->get_logger(), msg);
    resp->message = msg;
    resp->success = false;
    return true;
  }

  resp->success = true;
  return true;
}

bool BTServiceServer::resume_clbk(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
  std::shared_ptr<std_srvs::srv::Trigger::Response> resp)
{
  if (!tree_->isExecutorThreadActive()) {
    std::string msg{"Cannot resume when executor is not running!"};
    RCLCPP_WARN_STREAM(node_->get_logger(), msg);
    resp->message = msg;
    resp->success = false;
    return true;
  }

  if (!tree_->resume()) {
    std::string msg{"Cannot resume when tree is not paused or in manual mode!"};
    resp->message = msg;
    resp->success = false;
    return true;
  }
  resp->success = true;
  return true;
}

bool BTServiceServer::stop_clbk(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
  std::shared_ptr<std_srvs::srv::Trigger::Response> resp)
{
  if (!tree_->isExecutorThreadActive()) {
    std::string msg{"Cannot stop when executor is not running!"};
    RCLCPP_WARN_STREAM(node_->get_logger(), msg);
    resp->message = msg;
    resp->success = false;
    return true;
  }
  tree_->stop();
  resp->success = true;
  return true;
}

bool BTServiceServer::manual_tick_clbk(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
  std::shared_ptr<std_srvs::srv::Trigger::Response> resp)
{
  if (!tree_->isExecutorThreadActive()) {
    std::string msg{"Cannot tick when executor is not running!"};
    RCLCPP_WARN_STREAM(node_->get_logger(), msg);
    resp->message = msg;
    resp->success = false;
    return true;
  }

  if (!tree_->manualTick()) {
    std::string msg{"Cannot manual tick tree."};
    resp->message = msg;
    resp->success = false;
    return true;
  }

  resp->success = true;
  return true;
}

void BTServiceServer::loop_timer_clbk()
{
  auto msg = std_msgs::msg::String();
  msg.data = tree_->getStateString(tree_->getExecutorState());
  bt_state_publisher_->publish(msg);
}

}  // namespace bt_service_server
