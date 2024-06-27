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
        output="screen",
        parameters=[
            LaunchConfiguration('config_file_path').perform(context),
            {"behavior_tree_definitions_path" : LaunchConfiguration('behavior_tree_definitions_path').perform(context),
            "behavior_trees_path" : LaunchConfiguration('behavior_trees_path').perform(context)
            }
        ],
        emulate_tty=True
    )

    service_server = Node(
        package="bt_demo_skills",
        executable="service_server",
        name="service_server",
        output="screen"
    )
    return [bt_executor_node, service_server]

def generate_launch_description():    
    return LaunchDescription([
        DeclareLaunchArgument('config_file_path', 
            default_value=os.path.join(get_package_share_directory("bt_demo_skills"), "config", "default.yaml"), 
            description="Path to the node configuration file."),
        DeclareLaunchArgument('behavior_tree_definitions_path', 
            default_value=os.path.join(get_package_share_directory("bt_demo_skills"), "config", "trees/behavior_trees.yaml"), 
            description="Path to the behavior trees configuration file."),
        DeclareLaunchArgument('behavior_trees_path', 
            default_value=os.path.join(get_package_share_directory("bt_demo_skills"), "config", "trees/"), 
            description="Path to the behavior trees configuration file."),
        OpaqueFunction(function = launch_setup)
    ])
