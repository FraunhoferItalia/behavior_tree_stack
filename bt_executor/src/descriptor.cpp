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
// Author: Tobit Flatscher

#include "bt_executor/descriptor.h"

#include <behavior_tree_msgs/parsing_utils.h>
#include <behaviortree_cpp/basic_types.h>
#include <behaviortree_cpp/blackboard.h>

#include <cstdint>
#include <map>
#include <ostream>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <vector>

namespace behavior_tree_executor
{

Descriptor::Descriptor(std::string const & name, BT::Blackboard::Ptr & blackboard)
: name_{name}, blackboard_{blackboard}
{
  return;
}

Descriptor::Descriptor(std::string const & name, MapType const & map) noexcept
: name_{name}, blackboard_{createBlackboard(map)}
{
  return;
}

Descriptor::Descriptor(behavior_tree_msgs::msg::BehaviorTreeDescriptor const & msg) noexcept
{
  name_ = msg.behavior_tree_path;
  blackboard_ = createBlackboard(convertToMap(msg));
  return;
}

Descriptor::MapType Descriptor::convertToMap(
  behavior_tree_msgs::msg::BehaviorTreeDescriptor const & msg) noexcept
{
  MapType map{};
  for (auto const & m : msg.blackboard.map) {
    map[m.key] = m.value;
  }
  return map;
}

std::vector<behavior_tree_msgs::msg::KeyValuePair> Descriptor::convertToKeyValueVec(
  MapType const & map) noexcept
{
  std::vector<behavior_tree_msgs::msg::KeyValuePair> key_value_map{};
  for (auto const & m : map) {
    auto const & key{m.first};
    auto const & value{m.second};

    behavior_tree_msgs::msg::KeyValuePair key_value{};
    key_value.key = key;
    key_value.value = value;
    key_value_map.push_back(key_value);
  }
  return key_value_map;
}

BT::Blackboard::Ptr Descriptor::createBlackboard(
  MapType const & map, BT::Blackboard::Ptr const parent_blackboard)
{
  auto blackboard{BT::Blackboard::create(parent_blackboard)};
  for (auto const & m : map) {
    auto const & key{m.first};
    auto const & value{m.second};
    blackboard->createEntry(key, BT::InputPort<std::string>(key).second);
    blackboard->set(key, value);
  }
  return blackboard;
}

Descriptor::MapType Descriptor::toMap(BT::Blackboard::Ptr const blackboard)
{
  auto const keys{blackboard->getKeys()};
  MapType map{};
  for (auto const & k : keys) {
    std::string const key{BT::convertFromString<std::string>(k)};
    std::string const value{blackboard->get<std::string>(key)};
    map[key] = value;
  }
  return map;
}

behavior_tree_msgs::msg::BehaviorTreeDescriptor Descriptor::toMsg(
  std::string const & name, BT::Blackboard::Ptr const blackboard)
{
  auto const map{toMap(blackboard)};
  behavior_tree_msgs::msg::BehaviorTreeDescriptor bt_descriptor{};
  bt_descriptor.behavior_tree_path = name;
  for (auto const & m : map) {
    auto const & key = m.first;
    auto const & value = m.second;
    behavior_tree_msgs::msg::KeyValuePair key_value{};
    key_value.key = key;
    key_value.value = value;
    bt_descriptor.blackboard.map.push_back(key_value);
  }
  return bt_descriptor;
}

Descriptor::MapType Descriptor::toMap() const { return toMap(blackboard_); }

behavior_tree_msgs::msg::BehaviorTreeDescriptor Descriptor::toMsg() const
{
  return toMsg(name_, blackboard_);
}

std::string Descriptor::getName() const noexcept { return name_; }

BT::Blackboard::Ptr Descriptor::getBlackboard() const noexcept { return blackboard_; }

std::ostream & operator<<(std::ostream & os, Descriptor const & d) noexcept
{
  os << "Behavior tree name " << d.name_ << " with blackboard:" << std::endl;
  auto const keys{d.blackboard_->getKeys()};
  for (auto const & k : keys) {
    std::string const key{BT::convertFromString<std::string>(k)};
    std::string const value_type{BT::demangle(d.blackboard_->entryInfo(key)->type())};
    std::string const value{d.blackboard_->get<std::string>(key)};
    os << "- Key: " << key << ", value_type: " << value_type << ", value: " << value << std::endl;
  }
  return os;
}

}  // namespace behavior_tree_executor
