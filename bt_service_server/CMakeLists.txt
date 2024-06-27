cmake_minimum_required(VERSION 3.5)
project(bt_service_server)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake CACHE INTERNAL "" FORCE)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(bt_executor REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(ZeroMQ REQUIRED)

if(ZeroMQ_FOUND)
  add_definitions(-DZMQ_FOUND)
endif()

###########
## Build ##
###########

include_directories(
  include
)

set(LIBRARY_NAME ${PROJECT_NAME})

set(dependencies
  rclcpp 
  rclpy
  bt_executor
  behaviortree_cpp
  std_srvs
  std_msgs
)

add_library(${LIBRARY_NAME} SHARED
  src/bt_service_server.cpp 
)

add_executable(bt_service_server_node src/bt_service_server_node.cpp src/bt_service_server.cpp)
ament_target_dependencies(bt_service_server_node ${dependencies})
target_link_libraries(bt_service_server_node ${LIBRARY_NAME} ${bt_executor_LIBRARIES})
ament_target_dependencies(${LIBRARY_NAME} ${dependencies})

if(ZeroMQ_FOUND)
  target_link_libraries(bt_service_server_node ${ZeroMQ_LIBRARIES} ${bt_executor_LIBRARIES})
endif()

install(TARGETS bt_service_server
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin)

install(TARGETS bt_service_server_node
	RUNTIME DESTINATION lib/${LIBRARY_NAME})

install(DIRECTORY include/ DESTINATION include/)

install(DIRECTORY
  config
  launch
  DESTINATION 
  share/${PROJECT_NAME}
)

# Test python
ament_python_install_package(test)

install(PROGRAMS
test/test_bt_service_server.py
DESTINATION 
lib/${PROJECT_NAME} 
)

ament_package()



