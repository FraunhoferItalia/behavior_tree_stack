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

#ifndef BehaviorTreeExecutor_H
#define BehaviorTreeExecutor_H

#pragma once

#include <yaml-cpp/emitter.h>
#include <yaml-cpp/yaml.h>

#include <rclcpp/rclcpp.hpp>

#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/loggers/bt_cout_logger.h"
#include "behaviortree_cpp/loggers/bt_file_logger_v2.h"
#include "behaviortree_cpp/loggers/bt_minitrace_logger.h"
#include "behaviortree_cpp/loggers/groot2_publisher.h"
#include "bt_executor/descriptor.h"

namespace behavior_tree_executor
{
class BehaviorTreeExecutor;

typedef std::shared_ptr<behavior_tree_executor::BehaviorTreeExecutor> Ptr;
/**
 * @brief A behavior tree executor implemented through a simple sate machine.
 *
 */
class BehaviorTreeExecutor
{
public:
  enum ExecutorState {
    STOP = 0,
    RUNNING = 1,
    PAUSED = 2,
    MANUAL_MODE = 3,
    LOADING = 4,
  };

  std::shared_ptr<rclcpp::Node> node_;

  BehaviorTreeExecutor(std::shared_ptr<rclcpp::Node> node);

  virtual ~BehaviorTreeExecutor()
  {
    stop_flag_ = true;
    if (executor_thread_.joinable()) {
      executor_thread_.join();
    }
  };
  /**
   * @brief Run the default tree that has been read-in under var default_tree_name_.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool runDefaultTree();
  /**
   * @brief runs the behavior tree, loadTrees() has to be called before.
   *
   * @param tree_name the xml file name of the desired behavior tree to run
   * @return true if successful,
   * @return false otherwise.
   */
  bool runTree(const std::string & tree_name);
  /**
   * @brief stops the behavior tree execution thread and calls tree->halt().
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool stop();
  /**
   * @brief pauses the ticking of a running tree. If an ActionNode is Running at the moment of the call,
   * the action is halted or if the node exposes an input signal control_signal, the pause signal is sent.
   * The tree will stay running.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool pause();
  /**
   * @brief resumes a running tree if it was paused. Pay attention that nodes may not support this due
   * to ROS-services and ROS-actions not beeing able to be paused by default.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool resume();
  /**
   * @brief ticks a tree once.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool manualTick();
  /**
   * @brief can be called to verify if the tree is paused.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool isPaused();
  /**
   * @brief indicates if a behavior tree thread is running. 
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool isRunning();
  /**
   * @brief indicates if a behavior tree thread is stopped. 
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool isStopped();
  /**
   * @brief indicates if a behavior tree thread is in manual mode. 
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool isManualMode();
  /**
   * @brief indicates if a behavior tree thread is active. This function may be used mainly for 
   * thread joining.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool isExecutorThreadActive();
  /**
   * @brief returns the name of the running tree.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  std::string getRunningTreeName();
  /**
   * @brief parses a yaml file and creates a YAML:Node class.
   *
   * @param path_to_file as the path to the yaml file.
   * @return std::optional<YAML::Node> if successful,
   * @return {} otherwise.
   */
  std::optional<YAML::Node> parseYamlFile(const std::string & path_to_file);
  /**
   * @brief Registers a tree passed as an xml string through the behavior tree factory.
   * 
   * @param xml_text the input tree as xml.
   * @return std::vector<std::string> the list of registered trees.
   */
  BT::Tree getTreeFromXml(const std::string & xml_text);
  /**
   * @brief Parses the trees_ map for a tree with tree_name.
   * 
   * @param tree_name the requested tree name.
   * @return std::string the XML as a string.
   */
  std::string getTreeAsXml(const std::string tree_name);
  /**
   * @brief addTreeToMap inserts tree into map.
   * 
   * @param tree_name Name of the tree to be inserted as key into trees_map.
   * @param tree  Shared pointer to the tree.
   */
  void addTreeToMap(const std::string tree_name, const std::shared_ptr<BT::Tree> tree);
  /**
   * @brief Registers a tree passed as xml into the factory and creates a blackboard from the map.
   * 
   * @param xml a string containing the behavior tree as xml.
   * @param map a key value map representing the blackboard.
   * @return true if successful,
   * @return false otherwise.
   */
  bool loadTree(const std::string & xml, std::map<std::string, std::string> map);
  /**
   * @brief Loads a behavior tree forwarded as Descriptor into the map of trees.
   * 
   * @param descriptor 
   * @return true if successful,
   * @return false otherwise.
   */
  bool loadTree(Descriptor const & descriptor);
  /**
   * @brief write an XML file to the indicated path
   * 
   * @param tree a shared_pointer to the tree.
   * @param path the target path.
   * @return true if successful,
   * @return false otherwise.
   */
  bool writeXMLfromTree(std::shared_ptr<BT::Tree> tree, std::string path);
  /**
   * @brief extracts the name of the behavior tree beeing the xml node "main_tree_to_execute".
   * 
   * @param xml the behavior tree as a string.
   * @return std::string the extracted name.
   */
  std::string extractNameFromXml(const std::string & xml);
  /**
   * @brief extracts the name from the behavior-tree descriptor.
   * 
   * @param descriptor 
   * @return std::string the name of the tree.
   */
  std::string extractNameFromDescriptor(Descriptor const & descriptor);
  /**
   * @brief loads the trees indicated on the map behavior_tree_definitions.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool loadTrees();
  /**
   * @brief loads the trees indicated on the map trees_map.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool loadTrees(const std::map<std::string, std::string> trees_map);
  /**
   * @brief indicates if the last tree failed.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool isLastTreeFailed();
  /**
   * @brief get the Ecexutor State object.
   *
   * @return ExecutorState
   */
  ExecutorState getExecutorState();
  /**
   * @brief get the State String object.
   *
   * @return std::string as the state of the executor.
   */
  std::string getStateString(ExecutorState state);
  /**
   * @brief checks if the tree_name is in the local map of trees.
   * 
   * @param tree_name 
   * @return true if successful,
   * @return false otherwise.
   */
  bool isTreeInMap(const std::string & tree_name);

  BT::BehaviorTreeFactory factory_;  // the behavior tree factory class.
private:
  /**
   * @brief Read params from ROS parameter server indicated in the default.yaml config file.
   *
   * @return true if successful,
   * @return false otherwise.
   */
  bool readParameters();
  /**
   * @brief activates the loggers if desired from parameters.
   *
   * @param tree_name
   * @return true if successful,
   * @return false otherwise.
   */
  bool activateLoggers(const std::string & tree_name);
  /**
   * @brief activates a desired tree from the behavior_tree_definitions map.
   *
   * @param tree_name
   * @return true if successful,
   * @return false otherwise.
   */
  bool activateTreeFromMap(const std::string & tree_name);
  /**
   * @brief the loop that is executed as an own thread and running a desired behavior tree 
   * in a state machine.
   *
   */
  void loop();

  std::unique_ptr<BT::StdCoutLogger> logger_cout_;  // the terminal logger.
  std::unique_ptr<BT::FileLogger2> logger_file_;    // the file logger, creats an .fbl file.
  std::unique_ptr<BT::MinitraceLogger>
    logger_minitrace_;  // the minitrace logger, creates a .json file.
  std::unique_ptr<BT::Groot2Publisher> logger_groot_;  // the groot2 logger for the realtime monitor

  std::map<std::string, std::shared_ptr<BT::Tree>> trees_;  // the map holding the trees defined in
                                                            // behavior_tree_definitions_.
  std::shared_ptr<BT::Tree> tree_;  // the pointer of the currently acitaved tree.
  BT::Blackboard::Ptr blackboard_;
  std::string active_tree_name_;

  std::atomic<bool> stop_flag_{false};  // flag that is set to stop a running thread.
  bool is_running_flag_{false};         // flag indicating if the thread is running.
  bool is_paused_{false};               // flag that indicates if tree is paused.

  ExecutorState state_;
  bool tree_failed_{false};  // flag indicating if the last running tree failed.
  bool is_control_signal_available_{
    false};                     // flag indicating that at least one node in the tree has a
                                // control_signal input.
  std::mutex mutex_;            // mutex for thread access safety.
  std::condition_variable cv_;  // conditional variable used in stop().
  std::condition_variable cv_tick_notifier_;  // conditional variable used in manualTick().
  std::thread executor_thread_;               // the thread that is spawned up to run a tree.

  std::map<std::string, std::string> behavior_tree_definitions_;

  // parameters
  bool is_logging_;                             // flag for logging traces to files.
  bool is_terminal_logging_;                    // flag for logging traces to files.
  std::string log_folder_name_;                 // folder to which log is saved.
  std::string behavior_tree_definitions_path_;  // path containing the xml defining the BT.
  std::string behavior_trees_path_;             // path to the behavior trees
  std::string run_behavior_tree_path_;          // run and load behavior tree imediately.
  int zmq_publisher_port_;                      // the port for the zmq publisher (Groot).
  int zmq_server_port_;                         // the port for the zmq server (Groot).
  std::string default_tree_name_;               // default tree name to run.
  std::string output_xml_;  // path to a output file to export the xml of a running tree.
  int loop_rate_;
  rclcpp::TimerBase::SharedPtr timer_;  // the loop_rate of the executor thread.
  std::chrono::duration<double, std::milli> loop_timer_duration_;  // the loop_rate duration.
};
}  // namespace behavior_tree_executor
#endif