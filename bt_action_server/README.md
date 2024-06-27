# BT Action Server

Author: [Michael Terzer](mailto:michael.terzer@fraunhofer.it) - Fraunhofer Italia (2022)

This package hosts several C++ classes that implement ROS2 action servers. The actions can be used to control the execution of a behavior tree.

The package comes with 3 nodes beeing the standard `bt_action_server_node`, the `load_bt_action_server_node` and the `send_bt_action_server_node`.The difference of the nodes is the action goal type.

The `bt_action_server_node` takes as input a String that contains the name of the desired behavior tree, that has been loaded at bootup of the bt_action_server.

The `load_bt_action_server_node` takes as action goal a `behavior_tree_msgs/LoadBehaviortree.action`, a behavior tree can be sent as a `BehaviorTreeDescriptor` to the action server.

The `send_bt_action_server_node`, takes as action goal a `behavior_tree_msgs/LoadBehaviortree.action`. In this case the whole xml content and a key-value map with the blackboard of the behavior tree can be sent to the action server. This is very useful for distributed systems, e.g. a pilot PC sending a mission in form of a behavior tree to a robot.

### How to run

#### BT Action Server

The **bt_action_server** can be run by:

```
ros2 launch bt_action_server bt_action_server.launch.py
```

You should see following output:

```
[/bt_action_server_node :  INFO]-[1641389431.796045178]: BehaviorTreeExecutor init successful..
[/bt_action_server_node :  INFO]-[1641389431.796932626]: Loading trees...
[/bt_action_server_node :  INFO]-[1641389431.796982219]: tree_1
[/bt_action_server_node :  INFO]-[1641389431.797007150]: bt_action_server/config/trees/tree_example.xml le.xml
[/bt_action_server_node :  INFO]-[1641389431.803617158]: Started subtree_action_server on namespace /bt_action_server_node
```

You can now trigger the execution of **tree_1** by calling the action with the dedicated **control_signal** for starting a tree:

### Load BT Action Server

The **load_bt_action_server** can be run by:

```
ros2 launch bt_action_server load_bt_action_server.launch.py
```

#### Send BT Action Server

The **send_bt_action_server** can be run by:

```
ros2 launch bt_action_server send_bt_action_server.launch.py
```

### Test

The `load_bt_action_server` and `send_bt_action_server` can be tested by testing scripts which are located under the folder `/test`.

### Pytest

The pytests for the bt_action_server can be run with:

```
python3 -m pytest src/bt_action_server/test/test_send_bt_action_server.py -s
```

**HINT:** Currently the output of pytest cannot be displayed due to [this](https://github.com/colcon/colcon-ros/issues/151) issue. If this becomes working in future you may use:

```
colcon test --event-handlers console_cohesion+ --pytest-args -s --packages-select bt_action_server
```
