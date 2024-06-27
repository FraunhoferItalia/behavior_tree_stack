#!/usr/bin/env python3

# Copyright 2022-2024 Fraunhofer Italia Research
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# Copyright 2022 Fraunhofer Italia Research. All Rights Reserved.

import os
import copy
import numpy as np
import math

import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient
import rospkg
import tf2_ros

from behavior_tree_msgs.msg import *
from behavior_tree_msgs.srv import *
from behavior_tree_msgs.action import SendBehaviorTree as SendBehaviorTreeAction


class SendBehaviorTreeTester(Node):
    def __init__(self, behavior_tree_path):
        super().__init__("behavior_tree_tester")
        self.get_logger().info("Starting SendBehaviorTreeTester")

        descriptor = BehaviorTreeXml()
        self._action_client = ActionClient(
            self, SendBehaviorTreeAction, "bt_action_server"
        )
        input_ports = {}
        input_ports["integer"] = ["10"]
        input_ports["double"] = ["10.0"]
        input_ports["bool"] = ["false"]
        input_ports["string"] = ["hello world"]

        for port in input_ports:
            key_value = KeyValuePair()
            key_value.key = port
            key_value.value = input_ports[port][0]
            descriptor.blackboard.map.append(key_value)

        with open(behavior_tree_path, "r") as file:
            descriptor.behavior_tree_xml = file.read()

        goal_msg = SendBehaviorTreeAction.Goal()
        goal_msg.tree = descriptor
        print(descriptor.behavior_tree_xml)

        while not self._action_client.wait_for_server(timeout_sec=1.0):
            self.get_logger().info("action not available, waiting again...")
        self.get_logger().info("Connected to server!")
        self.get_logger().info("Sending action...")
        self._send_goal_future = self._action_client.send_goal_async(goal_msg)
        self._send_goal_future.add_done_callback(self.goal_response_callback)

        self.result_recieved = False
        self.result = ""
        return

    def goal_response_callback(self, future):
        goal_handle = future.result()
        if not goal_handle.accepted:
            self.get_logger().info("Goal rejected!")
            return

        self.get_logger().info("Goal accepted!")

        self._get_result_future = goal_handle.get_result_async()
        self._get_result_future.add_done_callback(self.get_result_callback)

    def get_result_callback(self, future):
        result = future.result().result
        self.get_logger().info("Result: {0}".format(result.message))
        self.result_recieved = True
        self.result = result.success

    def is_result(self):
        return self.result_recieved

    def get_result(self):
        return self.result


def main(args=None):
    print("Starting behavior tree action client..")
    rclpy.init(args=args)
    node = SendBehaviorTreeTester()
    rclpy.spin(node)
    node.destroy_node()


if __name__ == "__main__":
    main()
