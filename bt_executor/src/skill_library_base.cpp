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

#include "bt_executor/skill_library_base.h"

#include <behaviortree_cpp/bt_factory.h>

#include <rclcpp/rclcpp.hpp>

namespace bt_skill
{

void SkillLibraryBase::load(BT::BehaviorTreeFactory & factory, BT::RosNodeParams & params) const
{
  load(factory, params);
  return;
}

}  // namespace bt_skill
