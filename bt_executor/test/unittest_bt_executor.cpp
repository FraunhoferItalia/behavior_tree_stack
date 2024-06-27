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

// gtest
#include <gtest/gtest.h>

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <rclcpp/rclcpp.hpp>

#include "bt_executor/bt_executor.h"

namespace behavior_tree_executor
{
// From https://answers.ros.org/question/398825/colcon-test-processes-under-test-stopped-before-tests-completed/
// This is a bit of a hack to make thread sanitizer ignore a race condition
// in the constructor of the rclcpp::Node
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
__attribute__((no_sanitize("thread")))
#endif
#endif
rclcpp::Node::SharedPtr
make_node(std::string const & name, rclcpp::NodeOptions const & options)
{
  return std::make_shared<rclcpp::Node>(name, options);
}

/**
 * @brief BehaviorTreeExecutor test fixture class
 *
 */
class BehaviorTreeExecutorTest : public ::testing::Test
{
public:
  BehaviorTreeExecutorTest() : node_(std::make_shared<rclcpp::Node>("gtest_bt_executor"))
  {
    RCLCPP_ERROR_STREAM(node_->get_logger(), "Constructor finished!");
  }

protected:
  rclcpp::Node::SharedPtr node_;
  BehaviorTreeExecutor * bte_;
  std::thread spinner_;
  std::atomic<bool> stop{false};

  void SetUp() override
  {
    bte_ = new BehaviorTreeExecutor(node_);
    spinner_ = std::thread(&BehaviorTreeExecutorTest::spin, this);
  }

  void TearDown() override
  {
    stop = true;
    spinner_.join();
    delete bte_;
  }

  void spin()
  {
    while (!stop) {
      rclcpp::spin_some(node_->get_node_base_interface());
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
};

/**
 * @brief Test the public interface of the class BehaviorTreeExecutor
 *
 */
TEST_F(BehaviorTreeExecutorTest, instanciation) { ASSERT_EQ(bte_->loadTrees(), true); }

TEST_F(BehaviorTreeExecutorTest, isRunningOnInit) { EXPECT_EQ(bte_->isRunning(), false); }

TEST_F(BehaviorTreeExecutorTest, runTreeWithoutLoadingMap)
{
  EXPECT_EQ(bte_->runTree("xxx"), false);
}

TEST_F(BehaviorTreeExecutorTest, runTreeNotInMap)
{
  bte_->loadTrees();
  EXPECT_EQ(bte_->runTree("xxx"), false);
}

TEST_F(BehaviorTreeExecutorTest, stopRunningTree)
{
  bte_->loadTrees();
  EXPECT_EQ(bte_->runTree("tree_2"), true);
  EXPECT_EQ(bte_->isRunning(), true);
  EXPECT_EQ(bte_->stop(), true);
}

TEST_F(BehaviorTreeExecutorTest, pause)
{
  bte_->loadTrees();
  bte_->runTree("tree_2");
  EXPECT_EQ(bte_->pause(), true);
}

TEST_F(BehaviorTreeExecutorTest, isPaused)
{
  bte_->loadTrees();
  bte_->runTree("tree_2");
  bte_->pause();
  EXPECT_EQ(bte_->isPaused(), true);
}

TEST_F(BehaviorTreeExecutorTest, resumeTree)
{
  bte_->loadTrees();
  bte_->runTree("tree_2");
  bte_->pause();
  EXPECT_EQ(bte_->resume(), true);
}

TEST_F(BehaviorTreeExecutorTest, isRunningTree)
{
  bte_->loadTrees();
  EXPECT_EQ(bte_->runTree("tree_1"), true);
  while (bte_->isRunning()) {
    sleep(0.1);
  }
  EXPECT_EQ(bte_->isRunning(), false);
}

TEST_F(BehaviorTreeExecutorTest, isLastTreeFailedInit)
{
  bte_->loadTrees();
  EXPECT_EQ(bte_->isLastTreeFailed(), false);
}

TEST_F(BehaviorTreeExecutorTest, isLastTreeFailed)
{
  bte_->loadTrees();
  bte_->runTree("tree_1"), true;
  while (bte_->isRunning()) {
    sleep(0.1);
  }
  EXPECT_EQ(bte_->isLastTreeFailed(), false);
}

TEST_F(BehaviorTreeExecutorTest, isLastTreeFailedSequence)
{
  bte_->loadTrees();
  bte_->runTree("tree_1"), true;
  while (bte_->isRunning()) {
    sleep(0.1);
  }
  EXPECT_EQ(bte_->isLastTreeFailed(), false);
  bte_->runTree("tree_2"), true;
  while (bte_->isRunning()) {
    sleep(0.1);
  }
  EXPECT_EQ(bte_->isLastTreeFailed(), false);
  bte_->runTree("tree_3"), true;
  while (bte_->isRunning()) {
    sleep(0.1);
  }
  EXPECT_EQ(bte_->isLastTreeFailed(), true);
}

/**
 * @brief Test scenario:
 * 1. load the trees
 * 2. run a tree
 * 3. while the tree is running, access class members as often as possible (while loop)
 */
TEST_F(BehaviorTreeExecutorTest, penetrateWhileRunningTree)
{
  bte_->loadTrees();
  EXPECT_EQ(bte_->runTree("tree_2"), true);
  while (bte_->isRunning()) {
    bte_->loadTrees();
  }
  EXPECT_EQ(bte_->runTree("tree_3"), true);
  while (bte_->isRunning()) {
    EXPECT_EQ(bte_->runTree("tree_1"), false);
  }
  EXPECT_EQ(bte_->isRunning(), false);
}

/**
 * @brief Try to run several trees on same instance
 *
 */
TEST_F(BehaviorTreeExecutorTest, runSeveralTrees)
{
  bte_->loadTrees();
  EXPECT_EQ(bte_->runTree("tree_2"), true);
  while (bte_->isRunning()) {
  }
  EXPECT_EQ(bte_->runTree("tree_3"), true);
  while (bte_->isRunning()) {
  }
  EXPECT_EQ(bte_->runTree("tree_1"), true);
  while (bte_->isRunning()) {
  }
  EXPECT_EQ(bte_->isRunning(), false);
}

/**
 * @brief State machine transitions
 *
 */
TEST_F(BehaviorTreeExecutorTest, fromRunningToPaused)
{
  bte_->loadTrees();
  bte_->runTree("tree_2");
  EXPECT_EQ(bte_->isRunning(), true);
  EXPECT_EQ(bte_->pause(), true);
  EXPECT_EQ(bte_->isPaused(), true);
}

TEST_F(BehaviorTreeExecutorTest, fromRunningToManual)
{
  bte_->loadTrees();
  bte_->runTree("tree_2");
  EXPECT_EQ(bte_->isRunning(), true);
  bte_->manualTick();
  EXPECT_EQ(bte_->isManualMode(), true);
}

TEST_F(BehaviorTreeExecutorTest, fromPausedToRunning)
{
  bte_->loadTrees();
  bte_->runTree("tree_2");
  EXPECT_EQ(bte_->isRunning(), true);
  bte_->pause();
  EXPECT_EQ(bte_->resume(), true);
  EXPECT_EQ(bte_->isRunning(), true);
}
TEST_F(BehaviorTreeExecutorTest, fromPausedToRunningSeveralTimes)
{
  bte_->loadTrees();
  bte_->runTree("tree_2");
  EXPECT_EQ(bte_->isRunning(), true);
  for (int i = 0; i < 4; i++) {
    bte_->pause();
    EXPECT_EQ(bte_->resume(), true);
    EXPECT_EQ(bte_->isRunning(), true);
    sleep(0.5);
  }
}

TEST_F(BehaviorTreeExecutorTest, fromPauseToStop)
{
  bte_->loadTrees();
  bte_->runTree("tree_2");
  bte_->pause();
  EXPECT_EQ(bte_->stop(), true);
  EXPECT_EQ(bte_->isStopped(), true);
  EXPECT_EQ(bte_->isRunning(), false);
  EXPECT_EQ(bte_->isPaused(), false);
  EXPECT_EQ(bte_->isManualMode(), false);
}

TEST_F(BehaviorTreeExecutorTest, executorStateOnInit)
{
  bte_->loadTrees();
  EXPECT_EQ(bte_->isStopped(), true);
  EXPECT_EQ(bte_->isRunning(), false);
  EXPECT_EQ(bte_->isPaused(), false);
  EXPECT_EQ(bte_->isManualMode(), false);
}

TEST_F(BehaviorTreeExecutorTest, fromManualToRunning)
{
  bte_->loadTrees();
  bte_->runTree("tree_2");
  bte_->manualTick();
  EXPECT_EQ(bte_->resume(), true);
  EXPECT_EQ(bte_->isRunning(), true);
}

TEST_F(BehaviorTreeExecutorTest, fromManualToStop)
{
  bte_->loadTrees();
  bte_->runTree("tree_2");
}

TEST_F(BehaviorTreeExecutorTest, getExecutorStateRunning)
{
  bte_->loadTrees();
  bte_->runTree("tree_1");
  EXPECT_EQ(bte_->getExecutorState(), BehaviorTreeExecutor::ExecutorState::RUNNING);
}

TEST_F(BehaviorTreeExecutorTest, getExecutorStatePaused)
{
  bte_->loadTrees();
  bte_->runTree("tree_1");
  bte_->pause();
  EXPECT_EQ(bte_->getExecutorState(), BehaviorTreeExecutor::ExecutorState::PAUSED);
}

TEST_F(BehaviorTreeExecutorTest, getExecutorStateManualMode)
{
  bte_->loadTrees();
  bte_->runTree("tree_1");
  bte_->manualTick();
  EXPECT_EQ(bte_->getExecutorState(), BehaviorTreeExecutor::ExecutorState::MANUAL_MODE);
}

TEST_F(BehaviorTreeExecutorTest, isExecutorThreadActive)
{
  bte_->loadTrees();
  bte_->runTree("tree_1");
  EXPECT_EQ(bte_->isExecutorThreadActive(), true);
}

TEST_F(BehaviorTreeExecutorTest, isExecutorThreadInActive)
{
  bte_->loadTrees();
  bte_->runTree("tree_1");
  bte_->stop();
  EXPECT_EQ(bte_->isExecutorThreadActive(), false);
}

TEST_F(BehaviorTreeExecutorTest, isExecutorThreadInActiveLoop)
{
  bte_->loadTrees();
  for (int i = 0; i++; i < 10) {
    bte_->runTree("tree_1");
    bte_->stop();
    EXPECT_EQ(bte_->isExecutorThreadActive(), false);
    sleep(0.2);
  }
}

TEST_F(BehaviorTreeExecutorTest, getTreeAsXmlNonsense)
{
  bte_->loadTrees();
  EXPECT_EQ(bte_->getTreeAsXml("nonsense"), "");
}

TEST_F(BehaviorTreeExecutorTest, getTreeAsXml)
{
  bte_->loadTrees();
  EXPECT_NE(bte_->getTreeAsXml("tree_1"), "");
}

TEST_F(BehaviorTreeExecutorTest, testException)
{
  auto xml_text = R"(
     <root BTCPP_format="4" >
         <BehaviorTree ID="MainTree">
            <Sequence>
              <SetBlackboard value="1;2" output_key="wrong_list" />
            </Sequence>
         </BehaviorTree>
     </root>
    )";
  std::shared_ptr<BT::Tree> tree;

  tree = std::make_shared<BT::Tree>(bte_->getTreeFromXml(xml_text));
  bte_->addTreeToMap("test_exception_tree", tree);
  bte_->runTree("test_exception_tree");
}

TEST_F(BehaviorTreeExecutorTest, testLoadTreeFromDescriptor)
{
  std::string package_share_directory = ament_index_cpp::get_package_share_directory("bt_executor");
  std::string tree_path = package_share_directory + "/config/trees/test_tree_1.xml";
  auto msg = behavior_tree_msgs::msg::BehaviorTreeDescriptor();
  msg.behavior_tree_path = tree_path;

  auto map = behavior_tree_msgs::msg::KeyValueMap();
  auto pair = behavior_tree_msgs::msg::KeyValuePair();
  pair.key = "key";
  pair.value = "value";
  map.map.push_back(pair);
  msg.blackboard = map;

  EXPECT_EQ(bte_->loadTree(msg), true);
}

TEST_F(BehaviorTreeExecutorTest, testLoadRunTreeFromDescriptor)
{
  std::string package_share_directory = ament_index_cpp::get_package_share_directory("bt_executor");
  std::string tree_path = package_share_directory + "/config/trees/test_subtree.xml";
  auto msg = behavior_tree_msgs::msg::BehaviorTreeDescriptor();
  msg.behavior_tree_path = tree_path;

  auto map = behavior_tree_msgs::msg::KeyValueMap();
  auto pair = behavior_tree_msgs::msg::KeyValuePair();
  pair.key = "key";
  pair.value = "10";
  map.map.push_back(pair);
  msg.blackboard = map;

  EXPECT_EQ(bte_->loadTree(msg), true);
  EXPECT_EQ(bte_->runTree("TestSubtree"), true);
}

}  // namespace behavior_tree_executor
