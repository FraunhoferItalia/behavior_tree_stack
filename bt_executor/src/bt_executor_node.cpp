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

#include <rclcpp/rclcpp.hpp>

#include "bt_executor/bt_manager.h"

using namespace behavior_tree_executor;

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node_ = std::make_shared<rclcpp::Node>("bt_executor_node");

  BehaviorTreeManager btm(node_);

  if (!btm.bt_executor->loadTrees()) {
    rclcpp::shutdown();
    return 0;
  }

  // runs the default tree, as specified in the config.yaml
  btm.bt_executor->runDefaultTree();

  rclcpp::spin(btm.bt_executor->node_);
  rclcpp::shutdown();

  return 0;
}
