#!/usr/bin/env python3

import os
from ament_index_python.packages import (
    get_package_share_directory,
)
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def launch_setup(context, *args, **kwargs):
    bt_executor_node = Node(
        package="bt_executor",
        executable="bt_executor_node",
        name="bt_executor_node",
        #prefix=["gdbserver localhost:3000"],
        #prefix=['xterm -e gdb -ex run --args'],
        output="screen",
        parameters=[
            LaunchConfiguration('config_file_path').perform(context),
            {"behavior_tree_definitions_path" : LaunchConfiguration('behavior_tree_definitions_path').perform(context),
            "behavior_trees_path" : LaunchConfiguration('behavior_trees_path').perform(context),
            "run_behavior_tree_path" : LaunchConfiguration('run_behavior_tree_path').perform(context)
            }
        ],
        emulate_tty=True
    )
    return [bt_executor_node]

def generate_launch_description():    
    return LaunchDescription([
        DeclareLaunchArgument('config_file_path', 
            default_value=os.path.join(get_package_share_directory("bt_executor"), "config", "default.yaml"), 
            description="Path to the node configuration file."),
        DeclareLaunchArgument('behavior_tree_definitions_path', 
            default_value=os.path.join(get_package_share_directory("bt_executor"), "config", "trees/behavior_trees.yaml"), 
            description="Path to the behavior trees configuration file."),
        DeclareLaunchArgument('behavior_trees_path', 
            default_value=os.path.join(get_package_share_directory("bt_executor"), "config", "trees/"), 
            description="Path to the behavior trees configuration file."),
        DeclareLaunchArgument('run_behavior_tree_path', 
            default_value="", 
            description="Path to a behavior tree to be loaded and executed imediately."),
        OpaqueFunction(function = launch_setup)
    ])
