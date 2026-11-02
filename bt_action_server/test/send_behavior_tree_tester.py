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


from bt_action_server.bt_sender import (
    BehaviorTreeSender,
)  # Import the BehaviorTreeSender class

import threading
import time
import contextvars


class SendBehaviorTreeTester(Node):
    def __init__(self, behavior_tree_path, input_ports):
        super().__init__("behavior_tree_tester")

        self.get_logger().info("Starting SendBehaviorTreeTester")
        self.behavior_tree_path = behavior_tree_path
        self.input_ports = input_ports

        self.bt_sender = BehaviorTreeSender(self)
        self.logger = self.get_logger()
        self.status = BehaviorTreeSender.Status.IDLE

        self.cancel_mission = self.bt_sender.cancel_mission

        # dispatch thread to send the behavior tree
        thread = threading.Thread(target=self.send_bt)
        thread.start()
        return

    def send_bt(self):
        try:
            self.bt_sender.load_and_send(self.behavior_tree_path, self.input_ports)
            while self.bt_sender.status in {
                BehaviorTreeSender.Status.STARTING,
                BehaviorTreeSender.Status.RUNNING,
            }:
                self.status = self.bt_sender.status
                time.sleep(0.1)

            while self.bt_sender.status in {
                BehaviorTreeSender.Status.CANCELLING,
            }:
                time.sleep(0.1)

            self.status = self.bt_sender.status
            if self.bt_sender.status == BehaviorTreeSender.Status.SUCCEEDED:
                self.logger.info(f"Mission succeeded: {self.bt_sender.result.message}")
            elif self.bt_sender.status == BehaviorTreeSender.Status.FAILED:
                self.logger.error(f"Mission failed: {self.bt_sender.result.message}")
            elif self.bt_sender.status == BehaviorTreeSender.Status.CANCELLED:
                self.logger.info("Mission was cancelled.")
            else:
                self.logger.warning("Mission ended with an unexpected status.")
        except Exception as e:
            self.logger.error(f"An error occurred: {e}")
        finally:
            print("Shutting down SendBehaviorTreeTester")
            return
