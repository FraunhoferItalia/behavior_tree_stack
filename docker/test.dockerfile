FROM fhi-git01.fraunhofer.it:5050/behavior-tree/behavior_tree_stack:devel

ENV ROS_WS_DIR="/behavior_tree_stack_ws"
ENV ROS_DISTRO="${ROS_DISTRO}"

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR ${ROS_WS_DIR}

SHELL ["/bin/bash", "-c"]
RUN mkdir -p $ROS_WS_DIR/src/behavior_tree_stack
COPY ./ ./src/behavior_tree_stack

# from https://github.com/ros-planning/navigation2/blob/96673e216705fe837e5c53ce6a8469f204da5262/Dockerfile#L127
ARG RUN_TESTS
ARG FAIL_ON_TEST_FAILURE

RUN source /opt/ros/humble/setup.bash && \
  colcon build --cmake-args -DBUILD_TESTING=ON

RUN if [ -n "$RUN_TESTS" ]; then \
  source install/setup.bash  && \
  colcon test --event-handlers console_cohesion+ --packages-select bt_executor && \
  colcon test-result \
  || ([ -z "$FAIL_ON_TEST_FAILURE" ] || exit 1) \
  fi

ENV DEBIAN_FRONTEND=dialog

ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]

