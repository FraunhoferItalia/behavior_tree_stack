#!/usr/bin/env python3


from enum import Enum, auto
import os
import time
from typing import Dict
import rclpy
from rclpy.node import Node
from rclpy.callback_groups import CallbackGroup
from rclpy.action import ActionClient
import xml.etree.ElementTree as ET

from behavior_tree_msgs.msg import BehaviorTreeDescriptor, KeyValuePair
from behavior_tree_msgs.action import SendBehaviorTree as SendBehaviorTreeAction


BT_EXECUTABLE_TEMPLATE = """ <?xml version="1.0"?>
<root
    BTCPP_format="4"
    main_tree_to_execute="{}">
</root> 
"""


def read_behavior_tree(behavior_tree_path: str) -> str:
    """Reads a behavior tree file and returns the XML content as a string resolving any include tags

    Args:
        behavior_tree_path (str): Path to the behavior tree file

    Returns:
        str: XML content of the behavior tree file
    """

    with open(behavior_tree_path, "r") as file:
        behavior_tree_xml = file.read()

    behavior_tree_folder = os.path.dirname(behavior_tree_path)
    et = ET.fromstring(behavior_tree_xml)
    include_tags = [c for c in et if c.tag == "include"]
    for tag in include_tags:
        et.remove(tag)
        if not tag.attrib["path"].startswith("/"):
            subtree_et = ET.fromstring(
                read_behavior_tree(
                    os.path.join(behavior_tree_folder, tag.attrib["path"])
                )
            )
            for bt in subtree_et:
                et.insert(0, bt)
    return ET.tostring(et, encoding="utf-8", method="xml").decode("utf-8")


class BehaviorTreeSender:
    class Status(Enum):
        IDLE = auto()
        STARTING = auto()
        RUNNING = auto()
        SUCCEEDED = auto()
        FAILED = auto()
        CANCELLING = auto()
        CANCELLED = auto()

    def __init__(
        self,
        node: Node,
        server_name: str = "bt_action_server",
        callback_group: CallbackGroup = None,
    ):
        self.node = node
        self.status = BehaviorTreeSender.Status.IDLE
        self._action_client = ActionClient(
            self.node,
            SendBehaviorTreeAction,
            server_name,
            callback_group=callback_group,
        )

    def load_and_send(
        self,
        behavior_tree_path: str,
        input_ports: Dict[str, str],
        timeout: float = float("inf"),
    ) -> bool:
        if not os.path.isfile(behavior_tree_path):
            self.node.get_logger().error(
                f"Behavior Tree file {behavior_tree_path} does not exist"
            )
            return
        self.node.get_logger().info(
            f"Loading Behavior Tree from file {behavior_tree_path}"
        )
        behavior_tree_xml = read_behavior_tree(behavior_tree_path)
        return self.send(behavior_tree_xml, input_ports, timeout)

    def send(
        self,
        behavior_tree_xml: str,
        input_ports: Dict[str, str],
        timeout: float = float("inf"),
    ) -> bool:
        goal_msg = SendBehaviorTreeAction.Goal()
        for key, value in input_ports.items():
            key_value = KeyValuePair()
            key_value.key = key
            if value is None:
                self.get_logger().warning(
                    f"Unable to add key-value pair to blackboard. Value at `{key}` is None"
                )
            key_value.value = str(value)
            goal_msg.tree.blackboard.map.append(key_value)

        goal_msg.tree.behavior_tree_xml = behavior_tree_xml

        start_time = time.time()
        while not self._action_client.server_is_ready():
            if time.time() - start_time > timeout:
                return False
            self.node.get_logger().info(
                f"Action Server '{self._action_client._action_name}' not available, waiting again..."
            )
            time.sleep(1)

        self.node.get_logger().info("Connected to server!")
        self.node.get_logger().info("Sending action...")

        self.status = BehaviorTreeSender.Status.STARTING

        self._send_goal_future = self._action_client.send_goal_async(
            goal_msg, feedback_callback=self.feedback_callback
        )
        self._send_goal_future.add_done_callback(self.goal_response_callback)
        return True

    def feedback_callback(self, msg):
        feedback = msg.feedback
        self.node.get_logger().info(f"Received feedback: {feedback}")

    def wait_for_server(self):
        return self._action_client.wait_for_server(timeout_sec=1.0)

    def goal_response_callback(self, future):
        goal_handle = future.result()
        if not goal_handle.accepted:
            self.node.get_logger().info("Goal rejected!")
            self.status = BehaviorTreeSender.Status.IDLE
            return

        self.node.get_logger().info("Goal accepted!")

        self.status = BehaviorTreeSender.Status.RUNNING
        self.goal_handle = goal_handle

        self._get_result_future = self.goal_handle.get_result_async()
        self._get_result_future.add_done_callback(self.get_result_callback)
        return

    def get_result_callback(self, future):
        self.result = future.result().result
        if self.result.success:
            self.status = BehaviorTreeSender.Status.SUCCEEDED
        elif not self.result.success:
            self.status = BehaviorTreeSender.Status.FAILED
            if self.result.message == "Goal canceled, behavior tree stopped!":
                self.node.get_logger().info("Goal was successfully canceled.")
                self.status = BehaviorTreeSender.Status.CANCELLED

    def cancel_mission(self):
        if not self.status == BehaviorTreeSender.Status.RUNNING:
            self.node.get_logger().info(
                "Cannot cancel since mission does not appear to be running..."
            )
            return
        self.node.get_logger().info("Canceling mission...")
        if self.goal_handle:
            cancel_handle = self.goal_handle.cancel_goal_async()
            cancel_handle.add_done_callback(self.goal_canceled_callback)

    def goal_canceled_callback(self, cancel_handle):
        cancel_response = cancel_handle.result()
        if cancel_response.return_code == 0:
            print("Cancel request accepted")
            self.status = BehaviorTreeSender.Status.CANCELLING
            # print(f"Goals being canceled: {cancel_response.goals_canceling}")
        elif cancel_response.return_code == 1:
            print("Cancel request rejected")
        elif cancel_response.return_code == 2:
            print("Unknown goal ID")
        elif cancel_response.return_code == 3:
            print("Goal already terminated")

        # self.status = BehaviorTreeSender.Status.CANCELLED
        # self.mission_running = False

    def send_and_wait(
        self, behavior_tree_xml: str, input_ports: Dict[str, str], spin: bool = False
    ) -> bool:
        self.send(behavior_tree_xml, input_ports)
        while rclpy.ok() and self.status in {
            BehaviorTreeSender.Status.STARTING,
            BehaviorTreeSender.Status.RUNNING,
        }:
            if spin:
                rclpy.spin_once(self.node)
            else:
                time.sleep(0.01)

        return self.status == BehaviorTreeSender.Status.SUCCEEDED
