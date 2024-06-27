## Behavior Tree Executor

Author: [Michael Terzer](mailto:michael.terzer@fraunhofer.it) - Fraunhofer Italia 2023

This package hosts several C++ classes for controlling a Behavior Tree executor thread and managing a skill library. It can be used to load a list of Behavior Trees, as well as to run, pause, stop, and manually tick them. Thie functionalities are very useful for debugging Behavior Trees.

The executor thread is implemented as a state-machine. The state-machine includes the states _RUNNING_, _STOP_, _PAUSED_, _LOADING_ and _MANUAL_MODE_.

#### Pause behavior

As soon as a tree is RUNNING and the pause() function is called, the tree is not ticked again until the resume() method is called.

#### Resume

If the tree is in MANUAL_MODE or PAUSED state, the tree can be resumed by calling resume().

#### Manual tick

The manual tick method ticks the tree once.

#### Stop

The stop() method terminated the executor thread. This will unload the current running tree and terminate the loggers. To restart a desired tree, the runTree() method must be called again.

### C++ API

The config file under `/config/default.yaml` can be used to load a list of trees which can then be executed by calling `runTree("tree_xy")`, where `tree_xy` is the key-name of the tree.

See the `unittest_bt_executor.cpp` file for more examples on the usage of the API.

### Configuration

Following parameters are needed to configure the node:

- `is_logging`: If true, logging to files which groot can open is enabled.
- `is_terminal_logging`: If true, terminal logging of the Node-Changes is enabled.
- `log_folder_name`: The directory under which log files get stored.
- `loop_rate`: The loop rate with which the tree is ticked.
- `zmq_publisher_port`: The zmq publisher port, needed for groot.
- `zmq_server_port`: The zmq server port, needed for groot.
- `output_xml`: Indicate a path to where a xml file should be written before execution. (This is handy for trees that are assembled from subtrees and shall be displayed in Groot.)

### Run

Run an example of the bt_executor_node:

```
ros2 launch bt_executor behavior_tree.launch.py
```

### Tests

This package can be tested with colcon test, the tests are implemented with gtest.

Build the tests:

```
colcon build --cmake-args -DBUILD_TESTING=ON
```

Run the tests:

```
colcon test --event-handlers console_cohesion+ --packages-select bt_executor
```

Run raw gtest with filter:

```
./build/bt_executor/gtest_bt_executor --gtest_filter="BehaviorTreeExecutorTest.testException*"
```
