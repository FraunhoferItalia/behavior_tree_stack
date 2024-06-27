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

import std_srvs.srv
from std_srvs.srv import Trigger
from std_msgs.msg import String
from behavior_tree_msgs.srv import String as String_srv


@pytest.mark.launch_test
def generate_test_description():

    bt_service_server_node = launch_ros.actions.Node(
        package="bt_service_server",
        executable="bt_service_server_node",
        name="send_bt_service_server_node",
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
                    get_package_share_directory("bt_service_server"),
                    "config",
                    "default.yaml",
                ),
                description="Path to the node configuration file.",
            ),
            DeclareLaunchArgument(
                "behavior_tree_definitions_path",
                default_value=os.path.join(
                    get_package_share_directory("bt_service_server"),
                    "config",
                    "trees/behavior_trees.yaml",
                ),
                description="Path to the behavior trees configuration file.",
            ),
            DeclareLaunchArgument(
                "behavior_trees_path",
                default_value=os.path.join(
                    get_package_share_directory("bt_service_server"), "config", "trees/"
                ),
                description="Path to the behavior trees configuration file.",
            ),
            bt_service_server_node,
            service_server,
            # Start tests right away - no need to wait for anything
            launch_testing.actions.ReadyToTest(),
        ]
    )


class MinimalListener(Node):
    def __init__(self):
        super().__init__("minimal_listener")
        self.subscription = self.create_subscription(
            String, "bt_state", self.listener_callback, 10
        )
        self.state = ""

    def listener_callback(self, msg):
        self.get_logger().info('BT service server state: "%s"' % msg.data)
        self.state = msg.data

    def get_state(self):
        return self.state


class SimpleServiceClient(Node):
    def __init__(self, service_type, namespace):
        super().__init__("minimal_client_async")
        self.cli = self.create_client(service_type, namespace)
        while not self.cli.wait_for_service(timeout_sec=1.0):
            self.get_logger().info("service not available, waiting again...")

    def fill_req(self, request):
        self.req = request

    def send_request(self):
        self.future = self.cli.call_async(self.req)
        rclpy.spin_until_future_complete(self, self.future)
        return self.future.result()


class TestFixture(unittest.TestCase):
    def test_play_default_tree(self, proc_output):
        rclpy.init()
        minimal_client = SimpleServiceClient(Trigger, "play_default_tree")
        minimal_listener = MinimalListener()
        req = Trigger.Request()
        minimal_client.fill_req(req)
        response = minimal_client.send_request()
        run = True
        while run:
            rclpy.spin_once(minimal_listener)
            if minimal_listener.get_state() == "STOP":
                run = False

        assert response.success == True

    def test_play_specific_tree(self, proc_output):
        minimal_client = SimpleServiceClient(String_srv, "play_specific")
        minimal_listener = MinimalListener()
        req = String_srv.Request()
        req.str = "tree_2"
        minimal_client.fill_req(req)
        response = minimal_client.send_request()
        run = True
        while run:
            rclpy.spin_once(minimal_listener)
            if minimal_listener.get_state() == "STOP":
                run = False

        assert response.success == True
