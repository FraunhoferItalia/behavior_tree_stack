import os
import sys
import time
import unittest
import uuid

import launch
from launch.launch_service import LaunchService
import launch_ros
import launch_ros.actions
import launch_testing.actions
from launch_testing.io_handler import ActiveIoHandler
import launch_testing_ros

import os
from ament_index_python.packages import (
    get_package_share_directory,
)
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

import pytest

import rclpy
from rclpy.node import Node

import std_msgs.msg
from std_msgs.msg import String

from bt_action_server.bt_sender import (
    BehaviorTreeSender,
)  # Import the BehaviorTreeSender class

from send_behavior_tree_tester import SendBehaviorTreeTester


import numpy as np
from action_msgs.msg import GoalInfo
from action_msgs.srv import CancelGoal
from unique_identifier_msgs.msg import UUID
from builtin_interfaces.msg import Time

from threading import Thread
from rclpy.executors import MultiThreadedExecutor
from launch_testing.asserts import assertSequentialStdout


@pytest.mark.launch_test
def generate_test_description():

    bt_action_server_node = launch_ros.actions.Node(
        package="bt_action_server",
        executable="send_bt_action_server_node",
        name="send_bt_action_server_node",
        output="screen",
        emulate_tty=True,
    )

    service_server = launch_ros.actions.Node(
        package="bt_demo_skills",
        executable="service_server",
        name="service_server",
        output="screen",
        emulate_tty=True,
    )

    return launch.LaunchDescription(
        [
            DeclareLaunchArgument(
                "config_file_path",
                default_value=os.path.join(
                    get_package_share_directory("bt_action_server"),
                    "config",
                    "default.yaml",
                ),
                description="Path to the node configuration file.",
            ),
            DeclareLaunchArgument(
                "behavior_trees_path",
                default_value=os.path.join(
                    get_package_share_directory("bt_action_server"), "config", "trees/"
                ),
                description="Path to the behavior trees configuration file.",
            ),
            bt_action_server_node,
            service_server,
            # Start tests right away - no need to wait for anything
            launch_testing.actions.ReadyToTest(),
        ]
    )


@launch_testing.post_shutdown_test()
class TestProcessOutput(unittest.TestCase):

    def test_exit_code(self, proc_info):
        # Check that all processes in the launch exit with code 0
        launch_testing.asserts.assertExitCodes(proc_info)


class Callback:
    def __init__(self):
        self.callback_msg = ""

    def callback(self, msg):
        self.callback_msg = msg.data


class TestPublishString(unittest.TestCase):
    def setUp(self):
        rclpy.init()
        self.rclpy_ctx = rclpy.Context()
        rclpy.init(context=self.rclpy_ctx)
        self.executor = MultiThreadedExecutor(context=self.rclpy_ctx, num_threads=3)

        self.node = Node("behavior_tree_test_fixture")
        self.logger = self.node.get_logger()
        self.logger.info("SetUp...")
        self.executor.add_node(self.node)

        spin_thread = Thread(target=self.executor.spin, daemon=True)
        spin_thread.start()

    def tearDown(self):
        self.logger.info("TearDown...")
        self.rclpy_ctx.shutdown()
        rclpy.shutdown()

    def test_publish_string(self, proc_output):
        self.logger.info("TEST Publish String from BT ACTION")

        path = "/behavior_tree_stack_ws/src/bt_action_server/test/tree/test_tree.xml"
        input_ports = {}
        input_ports["integer"] = ["10"]
        input_ports["double"] = ["10.0"]
        input_ports["bool"] = ["false"]
        input_ports["string"] = ["hello world"]

        self.bt_tester = SendBehaviorTreeTester(path, input_ports)

        self.executor.add_node(self.bt_tester)

        callback = Callback()
        self.subscription = self.node.create_subscription(
            String,
            "test",
            callback.callback,
            10,
        )

        while self.bt_tester.status in {
            BehaviorTreeSender.Status.IDLE,
            BehaviorTreeSender.Status.STARTING,
        }:
            self.logger.info("Waiting for Running...")
            time.sleep(0.1)

        self.logger.info("Callback received: " + callback.callback_msg)
        assert callback.callback_msg == "Hello world!"

    def test_cancel_from_service(self, proc_output):
        self.logger.info("TEST cancel action")

        path = "/behavior_tree_stack_ws/src/bt_action_server/test/tree/test_tree.xml"
        input_ports = {}
        input_ports["integer"] = ["10"]
        input_ports["double"] = ["10.0"]
        input_ports["bool"] = ["false"]
        input_ports["string"] = ["hello world"]

        self.bt_tester = SendBehaviorTreeTester(path, input_ports)

        self.executor.add_node(self.bt_tester)

        while self.bt_tester.status in {
            BehaviorTreeSender.Status.IDLE,
            BehaviorTreeSender.Status.STARTING,
        }:
            self.logger.info("Waiting for RUNNING...")
            time.sleep(0.1)

        self.bt_tester.cancel_mission()

        while self.bt_tester.status in {
            BehaviorTreeSender.Status.RUNNING,
            BehaviorTreeSender.Status.CANCELLING,
        }:
            self.logger.info("Waiting for for mission beeing cancelled...")
            time.sleep(0.1)

    def test_loop_success(self, proc_output):
        self.logger.info("TEST loop action")

        path = "/behavior_tree_stack_ws/src/bt_action_server/test/tree/test_tree.xml"
        input_ports = {}
        input_ports["integer"] = ["10"]
        input_ports["double"] = ["10.0"]
        input_ports["bool"] = ["false"]
        input_ports["string"] = ["hello world"]

        for t in range(3):
            self.logger.info("New loop...")
            self.bt_tester = SendBehaviorTreeTester(path, input_ports)
            self.executor.add_node(self.bt_tester)
            while self.bt_tester.status != BehaviorTreeSender.Status.SUCCEEDED:
                time.sleep(0.1)
                self.logger.info("Waiting mission to succeed...")
            self.logger.info("Mission succeeded...")

    def test_nonsense(self, proc_output):
        self.logger.info("TEST Nonsense action")

        path = "/behavior_tree_stack_ws/src/bt_action_server/test/tree/test_tree_nonsense.xml"
        input_ports = {}
        input_ports["integer"] = ["10"]
        input_ports["double"] = ["10.0"]
        input_ports["bool"] = ["false"]
        input_ports["string"] = ["hello world"]

        self.bt_tester = SendBehaviorTreeTester(path, input_ports)
        self.executor.add_node(self.bt_tester)
        while self.bt_tester.status in {
            BehaviorTreeSender.Status.IDLE,
            BehaviorTreeSender.Status.STARTING,
            BehaviorTreeSender.Status.RUNNING,
        }:
            time.sleep(0.1)
            self.logger.info("RUNNING...")

        assert self.bt_tester.status == BehaviorTreeSender.Status.FAILED
