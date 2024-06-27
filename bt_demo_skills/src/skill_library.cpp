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

#include "bt_demo_skills/skill_library.h"

#include <behavior_tree_msgs/parsing_utils.h>
#include <behaviortree_cpp/bt_factory.h>

#include "behaviortree_ros2/ros_node_params.hpp"
#include "bt_demo_skills/publish_string.h"
#include "bt_demo_skills/trigger_service.h"
#include "bt_executor/skill_library_base.h"

namespace bt_demo_skills
{

void SkillLibrary::load(BT::BehaviorTreeFactory & factory, BT::RosNodeParams & params) const
{
  bt_skill::SkillLibraryBase::registerNodeType<PublishString>(factory, params);
  bt_skill::SkillLibraryBase::registerNodeType<TriggerService>(
    factory, params, "bt_demo_skills::Trigger");
  return;
}

}  // namespace bt_demo_skills
#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(bt_demo_skills::SkillLibrary, bt_skill::SkillLibraryBase)
