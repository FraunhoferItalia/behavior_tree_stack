#!/usr/bin/env python3
import argparse
import sys
import signal
import rclpy
from rclpy.node import Node
from bt_action_server.bt_sender import (
    BehaviorTreeSender,
)  # Import the BehaviorTreeSender class


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "behavior_tree_path", help="Path of the BehaviorTree file to be loaded"
    )
    parser.add_argument(
        "-sn",
        "--server_name",
        help="Name of the action server to which the goal is to be sent",
        default="bt_executor_node",
        required=False,
    )
    known_args, unknown_args = parser.parse_known_args(sys.argv[1:])
    input_ports = {arg.split(":=")[0]: arg.split(":=")[1] for arg in unknown_args}

    rclpy.init()
    node = Node("behavior_tree_loader")
    bt_sender = BehaviorTreeSender(node)
    logger = node.get_logger()

    def handle_sigint(signal_num, frame):
        bt_sender.cancel_mission()

    signal.signal(signal.SIGINT, handle_sigint)

    try:
        bt_sender.load_and_send(known_args.behavior_tree_path, input_ports)
        while rclpy.ok() and bt_sender.status in {
            BehaviorTreeSender.Status.STARTING,
            BehaviorTreeSender.Status.RUNNING,
        }:
            rclpy.spin_once(node)

        if bt_sender.status == BehaviorTreeSender.Status.SUCCEEDED:
            logger.info(f"Mission succeeded: {bt_sender.result.message}")
        elif bt_sender.status == BehaviorTreeSender.Status.FAILED:
            logger.error(f"Mission failed: {bt_sender.result.message}")
        elif bt_sender.status == BehaviorTreeSender.Status.CANCELLED:
            logger.info("Mission was cancelled.")
        else:
            logger.warning("Mission ended with an unexpected status.")
    except Exception as e:
        logger.error(f"An error occurred: {e}")
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
