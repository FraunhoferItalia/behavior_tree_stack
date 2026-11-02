cmake_minimum_required(VERSION 3.8)
project(behavior_tree_msgs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Any.msg"
  "msg/BehaviorTreeDescriptor.msg"
  "msg/BehaviorTreeXml.msg"
  "msg/KeyAnyMap.msg"
  "msg/KeyAnyPair.msg"
  "msg/KeyValueMap.msg"
  "msg/KeyValuePair.msg"
  "msg/PreemptionPolicy.msg"
  "msg/SchedulingPolicy.msg"
  "msg/ActionExtender.msg"
  "srv/ListBehaviorTrees.srv"
  "srv/LoadBehaviorTrees.srv"
  "srv/LoadBehaviorTree.srv"
  "srv/String.srv"
  "action/LoadBehaviorTree.action"
  "action/SendBehaviorTree.action"
  "action/Tree.action"
  DEPENDENCIES
  builtin_interfaces
)

include_directories(include)

add_library(${PROJECT_NAME}_lib src/parsing_utils.cpp)

ament_export_targets(${PROJECT_NAME}_lib HAS_LIBRARY_TARGET)

install(
  DIRECTORY include/${PROJECT_NAME}
  DESTINATION include/
)

install(
  TARGETS ${PROJECT_NAME}_lib
  EXPORT ${PROJECT_NAME}_lib
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)


ament_package()