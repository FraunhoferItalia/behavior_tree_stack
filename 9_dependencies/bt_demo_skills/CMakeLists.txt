cmake_minimum_required(VERSION 3.5)
project(bt_demo_skills)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()


if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(bt_executor REQUIRED)
find_package(pluginlib REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(behaviortree_ros2 REQUIRED)

include_directories(
  include
  )
set(LIBRARY_NAME ${PROJECT_NAME})

set(DEPENDENCIES
  rclcpp
  std_msgs
  std_srvs
  bt_executor
  pluginlib
  behaviortree_cpp
  behaviortree_ros2
)

add_library(${LIBRARY_NAME} SHARED src/skill_library.cpp src/publish_string.cpp src/trigger_service.cpp)

ament_target_dependencies(${LIBRARY_NAME} ${DEPENDENCIES})

pluginlib_export_plugin_description_file(bt_executor plugins.xml)

add_executable(service_server test/service_server.cpp)
ament_target_dependencies(service_server ${DEPENDENCIES})

install(
  TARGETS ${LIBRARY_NAME} service_server
  EXPORT  ${LIBRARY_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(TARGETS service_server
  RUNTIME DESTINATION lib/${LIBRARY_NAME}
)

install(DIRECTORY
  config
  launch
  DESTINATION 
  share/${PROJECT_NAME}
)

install(DIRECTORY include/ DESTINATION include/)

ament_export_libraries(${library_name})
ament_package()
