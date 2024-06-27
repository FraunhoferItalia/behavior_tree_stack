FROM ros:humble-ros-base

ENV ROS_WS_DIR="/behavior_tree_stack_ws"
ENV ROS_DISTRO="${ROS_DISTRO}"

ENV DEBIAN_FRONTEND=noninteractive

WORKDIR ${ROS_WS_DIR}

SHELL ["/bin/bash", "-c"]

#RUN mkdir -p $ROS_WS_DIR/src/behavior_tree_stack
#ADD ./ ./src/behavior_tree_stack

RUN apt-get update -qq \
  && apt-get -y install qtbase5-dev \
  && apt-get -y install libqt5svg5-dev \
  && apt-get -y install clang \
  && apt-get -y install clang-format \
  && apt-get -y install gdb \
  && apt-get -y install xterm \
  && apt-get -y install libzmq3-dev \
  && apt-get -y install libboost-all-dev \
  && apt-get -y install terminator

RUN apt-get update -qq \
  && apt-get -y install ros-${ROS_DISTRO}-ros-testing \
  && apt-get -y install ros-${ROS_DISTRO}-pluginlib \
  && apt-get -y install ros-${ROS_DISTRO}-rqt-gui \
  && apt-get -y install ros-${ROS_DISTRO}-generate-parameter-library

# for groot2
RUN apt-get update -qq \
  && apt-get -y install libfuse2 

ENV DEBIAN_FRONTEND=dialog

COPY ".vscode" "${ROS_WS_DIR}/.vscode"
RUN sed -i -E "s/(ROS_DISTRO=\").*(\")/\1${ROS_DISTRO}\2/" ${ROS_WS_DIR}/.vscode/ros_source.sh
RUN echo "source ${ROS_WS_DIR}/.vscode/ros_source.sh" >> ~/.bashrc
RUN echo "alias rsource=' source ${ROS_WS_DIR}/.vscode/ros_source.sh'" >> ~/.bashrc

ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]

