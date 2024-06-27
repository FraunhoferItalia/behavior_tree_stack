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

#include "bt_executor/bt_executor.h"

#include <tinyxml2.h>

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <chrono>
#include <filesystem>  //https://stackoverflow.com/questions/39231363/fatal-error-filesystem-no-such-file-or-directory
#include <fstream>
#include <iostream>

#include "behaviortree_cpp/loggers/bt_cout_logger.h"
#include "behaviortree_cpp/loggers/bt_file_logger_v2.h"
#include "behaviortree_cpp/loggers/bt_minitrace_logger.h"
#include "behaviortree_cpp/loggers/groot2_publisher.h"
#include "behaviortree_cpp/xml_parsing.h"

namespace fs = std::filesystem;

namespace behavior_tree_executor
{
using ServiceMap = std::map<std::string, std::string>;
using namespace std::chrono_literals;

BehaviorTreeExecutor::BehaviorTreeExecutor(std::shared_ptr<rclcpp::Node> node)
: node_(node), state_(ExecutorState::STOP)
{
  if (!readParameters()) {
    RCLCPP_ERROR(node_->get_logger(), "Could not read parameters.");
    rclcpp::shutdown();
    return;
  }
  RCLCPP_INFO_STREAM(node->get_logger(), "BehaviorTreeExecutor init successful..");
}

bool BehaviorTreeExecutor::readParameters()
{
  std::string package_share_directory = ament_index_cpp::get_package_share_directory("bt_executor");

  node_->declare_parameter<bool>("is_logging", true);
  node_->declare_parameter<bool>("is_terminal_logging", true);
  node_->declare_parameter<std::string>("log_folder_name", package_share_directory + "/test/");
  node_->declare_parameter<std::string>(
    "behavior_tree_definitions_path",
    package_share_directory + "/config/trees/behavior_trees.yaml");
  node_->declare_parameter<std::string>(
    "behavior_trees_path", package_share_directory + "/config/trees/");
  node_->declare_parameter<std::string>("run_behavior_tree_path", "");
  node_->declare_parameter<int>("zmq_publisher_port", 1666);
  node_->declare_parameter<int>("zmq_server_port", 1667);
  node_->declare_parameter<std::string>("default_tree_name", "tree_1");
  node_->declare_parameter<int>("loop_rate", 10);
  node_->declare_parameter<std::string>("output_xml", "");

  rclcpp::Parameter is_logging_param_ = node_->get_parameter("is_logging");
  is_logging_ = is_logging_param_.as_bool();

  rclcpp::Parameter is_terminal_logging_param_ = node_->get_parameter("is_terminal_logging");
  is_terminal_logging_ = is_terminal_logging_param_.as_bool();

  rclcpp::Parameter log_folder_name_param_ = node_->get_parameter("log_folder_name");
  log_folder_name_ = log_folder_name_param_.as_string();

  rclcpp::Parameter behavior_tree_definitions_path_param_ =
    node_->get_parameter("behavior_tree_definitions_path");
  behavior_tree_definitions_path_ = behavior_tree_definitions_path_param_.as_string();

  rclcpp::Parameter behavior_trees_path_param_ = node_->get_parameter("behavior_trees_path");
  behavior_trees_path_ = behavior_trees_path_param_.as_string();

  rclcpp::Parameter zmq_publisher_port_param_ = node_->get_parameter("zmq_publisher_port");
  zmq_publisher_port_ = zmq_publisher_port_param_.as_int();

  rclcpp::Parameter zmq_server_port_param_ = node_->get_parameter("zmq_server_port");
  zmq_server_port_ = zmq_server_port_param_.as_int();

  rclcpp::Parameter default_tree_name_param_ = node_->get_parameter("default_tree_name");
  default_tree_name_ = default_tree_name_param_.as_string();

  rclcpp::Parameter loop_rate_param_ = node_->get_parameter("loop_rate");
  loop_rate_ = loop_rate_param_.as_int();

  rclcpp::Parameter output_xml_param_ = node_->get_parameter("output_xml");
  output_xml_ = output_xml_param_.as_string();

  auto config = parseYamlFile(behavior_tree_definitions_path_);
  RCLCPP_INFO_STREAM(node_->get_logger(), behavior_tree_definitions_path_);
  if (!config) {
    return false;
  }

  const YAML::Node tree_definitions = config.value()["behavior_tree_definitions"];

  for (YAML::const_iterator it = tree_definitions.begin(); it != tree_definitions.end(); ++it) {
    std::string key = it->first.as<std::string>();
    behavior_tree_definitions_.emplace(
      std::make_pair(key, behavior_trees_path_ + it->second.as<std::string>()));
  }

  rclcpp::Parameter run_behavior_tree_path_param_ = node_->get_parameter("run_behavior_tree_path");
  run_behavior_tree_path_ = run_behavior_tree_path_param_.as_string();
  std::string run_behavior_tree_name_;
  if (run_behavior_tree_path_ != "") {
    std::ifstream t(run_behavior_tree_path_);
    std::stringstream buffer;
    buffer << t.rdbuf();
    run_behavior_tree_name_ = extractNameFromXml(buffer.str());
    if (run_behavior_tree_name_ == "") return false;
    default_tree_name_ = run_behavior_tree_name_;
    behavior_tree_definitions_.emplace(run_behavior_tree_name_, run_behavior_tree_path_);
  }

  return true;
}

std::optional<YAML::Node> BehaviorTreeExecutor::parseYamlFile(const std::string & path_to_file)
{
  YAML::Node config;
  try {
    config = YAML::LoadFile(path_to_file);
  } catch (const std::runtime_error & e) {
    RCLCPP_ERROR_STREAM(node_->get_logger(), "Failed to parse YAML file: " << e.what());
    return {};
  }
  return config;
}

bool BehaviorTreeExecutor::loadTrees(const std::map<std::string, std::string> trees_map)
{
  behavior_tree_definitions_ = trees_map;
  return loadTrees();
}

std::string BehaviorTreeExecutor::getTreeAsXml(const std::string tree_name)
{
  std::string xml_models = BT::writeTreeNodesModelXML(factory_);
  if (trees_.count(tree_name)) {
    auto tree = trees_.find(tree_name)->second;
    return BT::WriteTreeToXML(*tree, false, false);
  }
  RCLCPP_ERROR_STREAM(node_->get_logger(), "Tree: " << tree_name << " not in map.");
  return {};
}

void BehaviorTreeExecutor::addTreeToMap(
  const std::string tree_name, const std::shared_ptr<BT::Tree> tree)
{
  RCLCPP_INFO_STREAM(node_->get_logger(), "Insert tree: " << tree_name << " into the map.");
  if (trees_.count(tree_name)) {
    //overwrite if already in map
    trees_[tree_name] = tree;
    return;
  }

  trees_.insert(std::make_pair(tree_name, tree));
  return;
}

BT::Tree BehaviorTreeExecutor::getTreeFromXml(const std::string & xml_text)
{
  return factory_.createTreeFromText(xml_text);
}

bool BehaviorTreeExecutor::writeXMLfromTree(std::shared_ptr<BT::Tree> tree, std::string path)
{
  std::cout << "----------- XML file  ----------\n"
            << BT::WriteTreeToXML(*tree, false, false)
            << "-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n ";

  std::ofstream bt_file;
  bt_file.open(path);
  bt_file << BT::WriteTreeToXML(*tree, false, false);
  bt_file.close();
}

std::string BehaviorTreeExecutor::extractNameFromXml(const std::string & xml)
{
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError error_result = doc.Parse(xml.c_str());

  if (error_result == tinyxml2::XML_SUCCESS) {
    tinyxml2::XMLElement * root = doc.FirstChildElement("root");
    if ((root != nullptr) && (root->Attribute("main_tree_to_execute") != nullptr)) {
      std::string const name{root->Attribute("main_tree_to_execute")};
      return name;
    }
    RCLCPP_ERROR_STREAM(node_->get_logger(), "main_tree_to_execute name empty!");
    return {};
  }
  RCLCPP_ERROR_STREAM(node_->get_logger(), "Was not possible to load xml file!");

  return {};
}

std::string BehaviorTreeExecutor::extractNameFromDescriptor(Descriptor const & descriptor)
{
  std::string const behaviortree_file{descriptor.getName()};
  tinyxml2::XMLDocument doc{};

  tinyxml2::XMLError error_result{doc.LoadFile(behaviortree_file.c_str())};

  if (error_result == tinyxml2::XML_SUCCESS) {
    tinyxml2::XMLElement * root = doc.FirstChildElement("root");
    if ((root != nullptr) && (root->Attribute("main_tree_to_execute") != nullptr)) {
      std::string const tree_to_execute_name{root->Attribute("main_tree_to_execute")};
      return tree_to_execute_name;
    }
    RCLCPP_ERROR_STREAM(node_->get_logger(), "main_tree_to_execute name empty!");
    return {};
  }
  RCLCPP_ERROR_STREAM(
    node_->get_logger(), "Was not possible to load file on path: " << behaviortree_file);

  return {};
}

bool BehaviorTreeExecutor::loadTree(const std::string & xml, std::map<std::string, std::string> map)
{
  std::string tree_to_execute_name{extractNameFromXml(xml)};
  RCLCPP_INFO_STREAM(
    node_->get_logger(),
    "Loading new behavior tree " << tree_to_execute_name << " from descriptor...");

  if (tree_to_execute_name.empty()) {
    return false;
  }

  try {
    factory_.registerBehaviorTreeFromText(xml);
  } catch (const std::exception & e) {
    RCLCPP_ERROR_STREAM(node_->get_logger(), e.what());
    return false;
  }

  auto tree = std::shared_ptr<BT::Tree>();

  blackboard_ = Descriptor::createBlackboard(map);

  if (!blackboard_->getKeys().empty()) {
    RCLCPP_INFO_STREAM(node_->get_logger(), "The request contains a blackboard:");
    for (auto key : blackboard_->getKeys()) {
      std::string name(key);
      if (auto any_ref = blackboard_->getAnyLocked(name)) {
        if (auto any_ptr = any_ref.get()) {
          if (any_ptr->isString()) {
            auto value = any_ptr->cast<std::string>();
            RCLCPP_INFO_STREAM(node_->get_logger(), "key: " << key << ", value: " << value);
          }
        }
      }
    }
  }

  try {
    tree = std::make_shared<BT::Tree>(factory_.createTree(tree_to_execute_name, blackboard_));
  } catch (...) {
    RCLCPP_ERROR_STREAM(
      node_->get_logger(),
      "Was not able to create behavior tree from file! Check if you registered all nodes and "
      "that "
      "all input- and output-ports are set correctly in your xml.");
    return false;
  }
  addTreeToMap(tree_to_execute_name, tree);
  return true;
}

bool BehaviorTreeExecutor::loadTree(Descriptor const & descriptor)
{
  RCLCPP_INFO_STREAM(
    node_->get_logger(),
    "Loading new behavior tree " << descriptor.getName() << " from descriptor...");

  std::string tree_to_execute_name{};

  tree_to_execute_name = extractNameFromDescriptor(descriptor);
  if (tree_to_execute_name.empty()) {
    return false;
  }

  std::string const behaviortree_file{descriptor.getName()};
  try {
    factory_.registerBehaviorTreeFromFile(behaviortree_file);

  } catch (const std::exception & e) {
    RCLCPP_ERROR_STREAM(node_->get_logger(), e.what());
    return false;
  }
  auto tree = std::shared_ptr<BT::Tree>();
  try {
    tree = std::make_shared<BT::Tree>(
      factory_.createTree(tree_to_execute_name, descriptor.getBlackboard()));
  } catch (...) {
    RCLCPP_ERROR_STREAM(
      node_->get_logger(),
      "Was not able to create behavior tree from file! Check if you registered all nodes!");
    return false;
  }

  addTreeToMap(tree_to_execute_name, tree);
  return true;
}

bool BehaviorTreeExecutor::loadTrees()
{
  if (isRunning()) {
    RCLCPP_INFO_STREAM(node_->get_logger(), "Not possible to load Trees while runnning...");
    return false;
  }
  RCLCPP_INFO_STREAM(node_->get_logger(), "Loading trees...");

  try {
    for (auto [first, second] : behavior_tree_definitions_) {
      RCLCPP_INFO_STREAM(node_->get_logger(), first);
      RCLCPP_INFO_STREAM(node_->get_logger(), second);
      auto tree = std::make_shared<BT::Tree>(factory_.createTreeFromFile(second));
      addTreeToMap(first, tree);
    }
  } catch (const std::exception & e) {
    RCLCPP_ERROR_STREAM(
      node_->get_logger(), "Was not able to create tree. Exception: " << e.what());
    return false;
  }
  if (!fs::is_directory(fs::file_status(fs::status(log_folder_name_)))) {
    RCLCPP_INFO_STREAM(node_->get_logger(), "Creating log directory: " << log_folder_name_);
    try {
      fs::create_directory(log_folder_name_);
    } catch (const std::exception & e) {
      RCLCPP_ERROR_STREAM(
        node_->get_logger(), "Was not able to create directory. Exception: " << e.what());
    }
  }
  return true;
}

bool BehaviorTreeExecutor::isTreeInMap(const std::string & tree_name)
{
  if (trees_.count(tree_name)) {
    return true;
  }
  RCLCPP_WARN_STREAM(node_->get_logger(), "Tree: " << tree_name << " not found in tree_names map!");
  if (trees_.size() > 0) {
    RCLCPP_WARN_STREAM(node_->get_logger(), "Loaded trees in map are:");
    for (auto t : trees_) {
      RCLCPP_WARN_STREAM(node_->get_logger(), t.first);
    }
    return false;
  }

  RCLCPP_WARN_STREAM(
    node_->get_logger(), "Map of trees is empty, call loadTrees() before trying to run a Tree!");
  return false;
}

bool BehaviorTreeExecutor::activateTreeFromMap(const std::string & tree_name)
{
  if (trees_.count(tree_name)) {
    tree_ = trees_.find(tree_name)->second;
    return true;
  }
  return false;
}

bool BehaviorTreeExecutor::activateLoggers(const std::string & tree_name)
{
  if (is_terminal_logging_) {
    RCLCPP_INFO_STREAM(node_->get_logger(), "Terminal logging is enabled.");
    // This logger prints state changes on console
    try {
      logger_cout_ = std::make_unique<BT::StdCoutLogger>(*tree_);
    } catch (const BT::LogicError & e) {
      RCLCPP_WARN_STREAM(node_->get_logger(), e.what());
      return false;
    }
  }
  if (is_logging_) {
    try {
      // This logger saves state changes on file
      RCLCPP_INFO_STREAM(
        node_->get_logger(),
        "Logging file to " << (log_folder_name_ + tree_name + ".btlog").c_str());
      logger_file_ = std::make_unique<BT::FileLogger2>(
        *tree_, (log_folder_name_ + tree_name + ".btlog").c_str());

      // This logger stores the execution time of each node_
      RCLCPP_INFO_STREAM(
        node_->get_logger(), "Logging file to " << (log_folder_name_ + tree_name + ".json").c_str()
                                                << " and storing execution time of each node_.");
      logger_minitrace_ = std::make_unique<BT::MinitraceLogger>(
        *tree_, (log_folder_name_ + tree_name + ".json").c_str());

      logger_groot_ = std::make_unique<BT::Groot2Publisher>(*tree_);
      std::string xml_models = BT::writeTreeNodesModelXML(factory_);
    } catch (const std::exception & e) {
      RCLCPP_WARN_STREAM(node_->get_logger(), e.what());
      return false;
    }
  }
  return true;
}

bool BehaviorTreeExecutor::isLastTreeFailed()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return tree_failed_;
}

bool BehaviorTreeExecutor::runDefaultTree() { return runTree(default_tree_name_); }

bool BehaviorTreeExecutor::runTree(const std::string & tree_name)
{
  if (isRunning()) {
    RCLCPP_INFO_STREAM(node_->get_logger(), "Stop tree before running a new one...");
    return false;
  }
  state_ = ExecutorState::STOP;
  if (!activateTreeFromMap(tree_name)) {
    RCLCPP_ERROR_STREAM(node_->get_logger(), "Not able to run tree: " << tree_name);
    return false;
  }
  if (!activateLoggers(tree_name)) {
    RCLCPP_ERROR_STREAM(node_->get_logger(), "Not able to activate loggers");
    return false;
  }
  if (executor_thread_.joinable()) {
    executor_thread_.join();
  }
  RCLCPP_INFO_STREAM(node_->get_logger(), "Start to run tree: " << tree_name);
  executor_thread_ = std::thread(&BehaviorTreeExecutor::loop, this);

  // block until loop isRunning or timeout
  std::unique_lock<std::mutex> lk(mutex_);
  while (cv_.wait_for(lk, std::chrono::milliseconds(500)) != std::cv_status::timeout) {
    sleep(0.2);
  }
  active_tree_name_ = tree_name;
  return true;
}

template <class Container>
const bool contains(const Container & container, const typename Container::value_type & element)
{
  return std::find(container.begin(), container.end(), element) != container.end();
}

BehaviorTreeExecutor::ExecutorState BehaviorTreeExecutor::getExecutorState()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return state_;
}

std::string BehaviorTreeExecutor::getStateString(ExecutorState state)
{
  std::string s{};
  switch (state) {
    case (ExecutorState::RUNNING):
      s = "RUNNING";
      break;
    case (ExecutorState::PAUSED):
      s = "PAUSED";
      break;
    case (ExecutorState::MANUAL_MODE):
      s = "MANUAL_MODE";
      break;
    case (ExecutorState::STOP):
      s = "STOP";
      break;
    default:
      s = "";
      break;
  }
  return s;
}

void BehaviorTreeExecutor::loop()
{
  RCLCPP_INFO_STREAM(node_->get_logger(), "Start BehaviorTreeExecutor thread...");
  {
    std::lock_guard<std::mutex> lock(mutex_);
    stop_flag_ = false;
  }
  BT::NodeStatus status = BT::NodeStatus::RUNNING;

  cv_.notify_all();
  tree_failed_ = false;
  is_paused_ = false;

  if (output_xml_ != "") {
    writeXMLfromTree(tree_, output_xml_);
  }
  state_ = ExecutorState::RUNNING;
  while (status == BT::NodeStatus::RUNNING && rclcpp::ok() && !stop_flag_) {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      switch (state_) {
        case (ExecutorState::RUNNING):
          try {
            status = tree_->tickExactlyOnce();  // propagates to all children
          } catch (const std::exception & e) {
            RCLCPP_ERROR_STREAM(
              node_->get_logger(), "Terminating execution of tree: " << active_tree_name_);
            RCLCPP_ERROR_STREAM(node_->get_logger(), "Exception: " << e.what());
            stop_flag_ = true;
          }
          break;
        case (ExecutorState::PAUSED):
          // do nothing
          break;
        case (ExecutorState::MANUAL_MODE):
          status = tree_->tickExactlyOnce();
          state_ = ExecutorState::PAUSED;  // switch to paused after ticking once
      }
    }
    rclcpp::sleep_for(10ms);
  }

  if (status == BT::NodeStatus::FAILURE) {
    tree_failed_ = true;
    RCLCPP_WARN_STREAM(node_->get_logger(), "BehaviorTree returned FAILURE");
  }

  // clean up
  std::lock_guard<std::mutex> lock(mutex_);
  state_ = ExecutorState::STOP;
  cv_.notify_one();
  stop_flag_ = false;

  // reset loggers
  logger_cout_.reset(nullptr);
  logger_file_.reset(nullptr);
  logger_minitrace_.reset(nullptr);
  logger_groot_.reset(nullptr);
  RCLCPP_INFO_STREAM(node_->get_logger(), "BehaviorTreeExecutor thread ended...");
}

bool BehaviorTreeExecutor::stop()
{
  RCLCPP_WARN_STREAM(node_->get_logger(), "Stop BehaviorTreeExecutor thread...");
  {
    std::lock_guard<std::mutex> lock(mutex_);
    tree_->haltTree();
    state_ = ExecutorState::STOP;
    stop_flag_ = true;
  }
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait_for(lock, std::chrono::seconds(1));
  }
  if (executor_thread_.joinable()) {
    executor_thread_.join();
  }
  return true;
}

bool BehaviorTreeExecutor::isRunning()
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (state_ == ExecutorState::RUNNING) {
    return true;
  }
  return false;
}

bool BehaviorTreeExecutor::isStopped()
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (state_ == ExecutorState::STOP) {
    return true;
  }
  return false;
}

std::string BehaviorTreeExecutor::getRunningTreeName()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return active_tree_name_;
}

bool BehaviorTreeExecutor::pause()
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (state_ != ExecutorState::PAUSED) {
    state_ = ExecutorState::PAUSED;
    return true;
  }
  RCLCPP_ERROR_STREAM(node_->get_logger(), "Not possible to pause if already paused...");
  return false;
}

bool BehaviorTreeExecutor::resume()
{
  RCLCPP_WARN_STREAM(node_->get_logger(), "Resuming tree...");
  std::lock_guard<std::mutex> lock(mutex_);
  if ((state_ == ExecutorState::PAUSED) || (state_ == ExecutorState::MANUAL_MODE)) {
    state_ = ExecutorState::RUNNING;
    return true;
  }
  RCLCPP_WARN_STREAM(
    node_->get_logger(), "Cannot resume, when tree is not paused or in manual mode...");
  return false;
}

// this is for for manual tick through ticking the root once.
bool BehaviorTreeExecutor::manualTick()
{
  if (isStopped()) {
    RCLCPP_WARN_STREAM(node_->get_logger(), "Cannot manual tick when tree is stopped.");
    return false;
  }
  RCLCPP_INFO_STREAM(node_->get_logger(), "BT Executor: Manual tick tree...");
  {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = ExecutorState::MANUAL_MODE;
  }
  std::lock_guard<std::mutex> lock(mutex_);
  return true;
}

bool BehaviorTreeExecutor::isPaused()
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (state_ == ExecutorState::PAUSED) {
    return true;
  }
  return false;
}

bool BehaviorTreeExecutor::isManualMode()
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (state_ == ExecutorState::MANUAL_MODE) {
    return true;
  }
  return false;
}

bool BehaviorTreeExecutor::isExecutorThreadActive()
{
  std::lock_guard<std::mutex> lock(mutex_);
  if (executor_thread_.joinable()) {
    return true;
  }
  return false;
}

}  // namespace behavior_tree_executor
