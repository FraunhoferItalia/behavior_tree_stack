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

#ifndef PUBLISH_STRING_SKILL_LIBRARY
#define PUBLISH_STRING_SKILL_LIBRARY
#pragma once

#include <behaviortree_cpp/action_node.h>
#include <behaviortree_cpp/basic_types.h>

#include <rclcpp/rclcpp.hpp>
#include <string>

#include "behaviortree_ros2/bt_topic_pub_node.hpp"
#include "std_msgs/msg/string.hpp"

namespace bt_demo_skills
{

/**\class PublishString
   * \brief
   *   Output a message over a string message on a given topic
  */
class PublishString : public BT::RosTopicPubNode<std_msgs::msg::String>
{
public:
  /**\fn PublishString
       * \brief
       *   Constructor
       * 
       * \param[in] nh
       *   The ROS node handle of the ROS node running the executor
       * \param[in] name
       *   The name of the corresponding XML tag to be expected
       * \param[in] conf
       *   The behaviour tree node configuration
      */
  PublishString(
    std::string const & name, BT::NodeConfig const & config, const BT::RosNodeParams & params);

  /**\fn providedPorts
       * \brief
       *   Functions for registering the inputs (ports) that the node expects from the XML entry
       * \warning
       *   Has to be defined by any child class!
       * 
       * \return
       *   A description of the expected input ports
      */
  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({BT::InputPort<std::string>("message")});
  }

protected:
  /**\fn parseMessage
       * \brief
       *   Function for parsing the input parameters and filling up the corresponding message for a particular leaf
       * \warning
       *   Has to be defined by any child class!
       * 
       * \param[in,out] msg
       *   The message to be send that is filled up with information inside this function
       * \return
       *   A Boolean variable signaling success or failure of the parsing of the input parameters
      */
  bool setMessage(std_msgs::msg::String & msg) override;

  std::shared_ptr<rclcpp::Node> nh_;
  std::string name_;
};

}  // namespace bt_demo_skills

#endif  // PUBLISH_STRING_SKILL_LIBRARY
