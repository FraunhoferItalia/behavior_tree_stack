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

#ifndef BehaviorTreeManager_H
#define BehaviorTreeManager_H

#pragma once

#include "bt_executor/bt_executor.h"
#include "bt_executor/bt_skill_manager.h"

namespace behavior_tree_executor
{

/**
 * This class manages the underlaying bt executor.
*/
class BehaviorTreeManager
{
public:
  BehaviorTreeManager(std::shared_ptr<rclcpp::Node> node);
  behavior_tree_executor::Ptr bt_executor;

private:
  BehaviorTreeSkillManager bt_skill_manager_;
};
}  // namespace behavior_tree_executor
#endif  //BehaviorTreeManager_H