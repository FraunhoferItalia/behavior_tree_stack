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
from behavior_tree_msgs.action import LoadBehaviorTree as LoadBehaviorTreeAction


class LoadBehaviorTreeTester(Node):
    def __init__(self):
        super().__init__("behavior_tree_tester")
        self.get_logger().info("Starting LoadBehaviorTreeTester")

        descriptor = BehaviorTreeDescriptor()
        self._action_client = ActionClient(
            self, LoadBehaviorTreeAction, "bt_action_server"
        )
        input_ports = {}
        input_ports["key"] = ["10"]

        for port in input_ports:
            key_value = KeyValuePair()
            key_value.key = port
            key_value.value = input_ports[port][0]
            descriptor.blackboard.map.append(key_value)
        descriptor.behavior_tree_path = (
            "/behavior_tree_stack_ws/src/bt_action_server/config/trees/test.xml"
        )

        goal_msg = LoadBehaviorTreeAction.Goal()
        goal_msg.tree = descriptor

        while not self._action_client.wait_for_server(timeout_sec=1.0):
            self.get_logger().info("action not available, waiting again...")
        self.get_logger().info("Connected to server!")
        self.get_logger().info("Sending action...")
        self._send_goal_future = self._action_client.send_goal_async(goal_msg)
        self._send_goal_future.add_done_callback(self.goal_response_callback)

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
        rclpy.shutdown()

    def read_behavior_tree(self, behavior_tree_path: str):
        with open(behavior_tree_path, "r") as file:
            behavior_tree_xml = file.read()

        behavior_tree_folder = os.path.dirname(behavior_tree_path)
        et = ET.fromstring(behavior_tree_xml)
        include_tags = [c for c in et if c.tag == "include"]
        for tag in include_tags:
            et.remove(tag)
            if not tag.attrib["path"].startswith("/"):
                subtree_et = ET.fromstring(
                    self.read_behavior_tree(
                        os.path.join(behavior_tree_folder, tag.attrib["path"])
                    )
                )
                for bt in subtree_et:
                    et.insert(0, bt)
        return ET.tostring(et, encoding="utf-8", method="xml").decode("utf-8")


def main(args=None):
    print("Starting behavior tree action client..")
    rclpy.init(args=args)
    node = LoadBehaviorTreeTester()
    rclpy.spin(node)
    node.destroy_node()


if __name__ == "__main__":
    main()
