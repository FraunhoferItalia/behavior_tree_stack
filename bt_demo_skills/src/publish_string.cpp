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

#include "bt_demo_skills/publish_string.h"

#include <behaviortree_cpp/action_node.h>
#include <behaviortree_cpp/basic_types.h>

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <string>

namespace bt_demo_skills
{

PublishString::PublishString(
  std::string const & name, BT::NodeConfig const & config, const BT::RosNodeParams & params)
: RosTopicPubNode<std_msgs::msg::String>{name, config, params}, name_{name}
{
  return;
}

bool PublishString::setMessage(std_msgs::msg::String & msg)
{
  std::string message{};

  if (!getInput<std::string>("message", message)) {
    RCLCPP_ERROR_STREAM(nh_->get_logger(), "Could not parse input parameter for message.");
    return false;
  }
  msg.data = message;
  return true;
}

}  // namespace bt_demo_skills
