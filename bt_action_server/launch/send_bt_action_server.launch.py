#!/usr/bin/env python3

import os
import sys
from tempfile import NamedTemporaryFile
import yaml
from ament_index_python.packages import (
    get_package_share_directory,
)
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def resolve_relative_paths(yaml_file):
    # Get the directory of the YAML file
    yaml_dir = os.path.dirname(yaml_file)

    # Load the YAML file
    with open(yaml_file, "r") as file:
        params = yaml.safe_load(file)

    def resolve_paths_recursive(data, upper_key=None):
        """Recursively resolve paths in the nested dictionary."""
        if isinstance(data, dict):
            for key, value in data.items():
                data[key] = resolve_paths_recursive(value, key)
        elif isinstance(data, list):
            # Handle lists in YAML (e.g., sequences of paths)
            return [resolve_paths_recursive(item, upper_key) for item in data]
        elif (
            upper_key
            and isinstance(data, str)
            and "path" in upper_key
            and not os.path.isabs(data)
        ):
            full_path = os.path.realpath(os.path.join(yaml_dir, data))
            if os.path.exists(full_path):
                return full_path
            else:
                print(f"File {full_path} does not exist")
                sys.exit(1)
        return data

    # Resolve paths for all keys

    with NamedTemporaryFile(mode="w", prefix="launch_params_", delete=False) as h:
        h.write(yaml.dump(resolve_paths_recursive(params)))

    return h.name


def launch_setup(context, *args, **kwargs):
    print(resolve_relative_paths(LaunchConfiguration("config_file").perform(context)))
    return [
        Node(
            package="bt_action_server",
            executable="send_bt_action_server_node",
            name="bt_executor_node",
            output="screen",
            parameters=[
                resolve_relative_paths(
                    LaunchConfiguration("config_file").perform(context)
                )
            ],
            emulate_tty=True,
        )
    ]


def generate_launch_description():
    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "config_file",
                default_value=os.path.join(
                    get_package_share_directory("bt_action_server"),
                    "config",
                    "default.yaml",
                ),
                description="Path to the node configuration file.",
            ),
            OpaqueFunction(function=launch_setup),
        ]
    )
