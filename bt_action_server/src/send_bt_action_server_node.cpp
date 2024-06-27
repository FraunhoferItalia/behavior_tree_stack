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
#include "bt_executor/bt_manager.h"

using namespace behavior_tree_executor;

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto bt_executor_node_ = std::make_shared<rclcpp::Node>("bt_executor_node_");

  std::shared_ptr<BehaviorTreeManager> bt_manager =
    std::make_shared<BehaviorTreeManager>(bt_executor_node_);

  if (!bt_manager->bt_executor->loadTrees()) {
    rclcpp::shutdown();
  }

  bt_action_server::SendBTActionServer bt_action_server(bt_manager->bt_executor, bt_executor_node_);

  rclcpp::spin(bt_executor_node_);
  rclcpp::shutdown();

  return 0;
}
