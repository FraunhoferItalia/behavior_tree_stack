# BT Service Server

Author: [Michael Terzer](mailto:michael.terzer@fraunhofer.it) - Fraunhofer Italia (2022)

This package hosts a class `BTServiceServer` that exposes several ROS2 services. These services can be used to control the execution of a behavior tree.

### Provided ROS-services:

- `/play` : std_msgs/Trigger: runs a Behavior Tree that has been declared through the `default_tree_name` parameter.
- `/play_specific` : behavior_tree_msgs/String: runs a Behavior Tree that matches the string forwarded through the service call.
- `/load_and_run_service_name` : behavior_tree_msgs/LoadBehaviorTree: loads a Behavior Tree that can be found at the path that is forwarded through the message.
- `/pause` : std_msgs/Trigger; pauses a runniong Behavior Tree.
- `/resume`: std_msgs/Trigger; resumes a paused Behavior Tree
- `/stop`: std_msgs/Trigger; stops the execution of a Behavior Tree.
- `/manual_tick`: std_msgs/Trigger; pauses a running Behavior Tree and ticks the Tree once.

### Configuration

The configuration of the node can be found under `/config/default.yaml`. Following parameters must be set:

- **bt_service_server_name**: The namespace of the service server under which the services are exposed.
- **play_default_tree_service_name**: The name of the play service.
- **play_specific_tree_service_name**:The name of the play_specific service, which takes a string as input.
- **pause_service_name**: The name of the pause service.
- **resume_service_name**: The name of the resume service.
- **stop_service_name**: The name of the stop service.
- **manual_tick_service_name**: The name of the manual tick service.
- **default_tree_name**: The name of the default tree, that is run when calling the play service.

### How to run

Start a test service server by running:

```
ros2 launch bt_service_server behavior_tree.launch.py
```

Once it is up and running, open another terminal and run:

```
ros2 service call /play std_srvs/srv/Trigger "{}"
```

The tree_1 should start. Since the services are not blocking, the state of the executor might be monitored through the topic `bt_state`. 

### Tests

The tests can be run by:

```
python3 -m pytest src/bt_service_server/test/test_bt_service_server.py -s
```
