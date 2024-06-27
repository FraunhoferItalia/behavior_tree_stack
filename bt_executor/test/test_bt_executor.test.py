# -*- coding: utf-8 -*-
import launch
from launch.actions import TimerAction
from launch.substitutions import PathJoinSubstitution
import launch_ros
import launch_testing
import os
import sys
import unittest


def generate_test_description():
    gtest_bt_executor = launch_ros.actions.Node(
        executable=launch.substitutions.PathJoinSubstitution(
            [
                launch.substitutions.LaunchConfiguration("test_binary_dir"),
                "gtest_bt_executor",
            ]
        ),
        output="screen",
    )

    return launch.LaunchDescription(
        [
            launch.actions.DeclareLaunchArgument(
                name="test_binary_dir",
                description="Binary directory of package "
                "containing test executables",
            ),
            TimerAction(period=2.0, actions=[gtest_bt_executor]),
            launch_testing.util.KeepAliveProc(),
            launch_testing.actions.ReadyToTest(),
        ]
    ), {
        "gtest_bt_executor": gtest_bt_executor,
    }

# from https://github.com/ros-controls/ros2_control/blob/0bdcd414c7ab8091f3e1b8d9b73a91c778388e82/joint_limits/test/joint_limits_rosparam.launch.py#L50
class TestBTExecutor(unittest.TestCase):
    def test_termination(self, gtest_bt_executor, proc_info):
        proc_info.assertWaitForShutdown(process=gtest_bt_executor, timeout=(40000))


@launch_testing.post_shutdown_test()
class TestBTExecutorTestAfterShutdown(unittest.TestCase):
    def test_exit_code(self, proc_info):
        # Check that all processes in the launch (in this case, there's just one) exit
        # with code 0
        launch_testing.asserts.assertExitCodes(proc_info)
