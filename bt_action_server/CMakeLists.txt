cmake_minimum_required(VERSION 3.5)
project(bt_action_server)

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
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(behavior_tree_msgs REQUIRED)
find_package(bt_executor REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(ament_cmake_pytest REQUIRED)

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
  rclcpp_action
  rclcpp_components
  behavior_tree_msgs
  bt_executor
  behaviortree_cpp
  std_srvs
  std_msgs
  ament_cmake_pytest
)

add_library(${LIBRARY_NAME} SHARED
  src/bt_action_server.cpp 
  src/load_bt_action_server.cpp 
  src/send_bt_action_server.cpp 
)

add_executable(bt_action_server_node src/bt_action_server_node.cpp src/bt_action_server.cpp)
target_link_libraries(bt_action_server_node ${LIBRARY_NAME} ${bt_executor_LIBRARIES})

target_compile_definitions(bt_action_server
  PRIVATE "ACTION_TUTORIALS_CPP_BUILDING_DLL")

add_executable(load_bt_action_server_node src/load_bt_action_server_node.cpp src/load_bt_action_server.cpp)
target_link_libraries(load_bt_action_server_node ${LIBRARY_NAME} ${bt_executor_LIBRARIES})

add_executable(send_bt_action_server_node src/send_bt_action_server_node.cpp src/send_bt_action_server.cpp)
target_link_libraries(send_bt_action_server_node ${LIBRARY_NAME} ${bt_executor_LIBRARIES})


ament_target_dependencies(${LIBRARY_NAME} ${dependencies})

if(ZeroMQ_FOUND)
  target_link_libraries(bt_action_server_node ${ZeroMQ_LIBRARIES} ${bt_executor_LIBRARIES})
  target_link_libraries(load_bt_action_server_node ${ZeroMQ_LIBRARIES} ${bt_executor_LIBRARIES})
  target_link_libraries(send_bt_action_server_node ${ZeroMQ_LIBRARIES} ${bt_executor_LIBRARIES})
endif()


install(TARGETS bt_action_server
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin)

install(TARGETS bt_action_server_node load_bt_action_server_node send_bt_action_server_node
	RUNTIME DESTINATION lib/${LIBRARY_NAME})

install(DIRECTORY include/ DESTINATION include/)

install(DIRECTORY
  config
  launch
  test
  DESTINATION 
  share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_cmake_pytest REQUIRED)
  set(_pytest_tests test/test_send_bt_action_server.py)
  foreach(test_path ${_pytest_tests})
    get_filename_component(_test_name ${test_path} NAME_WE)
    ament_add_pytest_test(
      ${_test_name}
      ${test_path}
      APPEND_ENV
      AMENT_PREFIX_INDEX=${ament_index_build_path}
      PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}
      TIMEOUT
      60
      WORKING_DIRECTORY
      "${CMAKE_SOURCE_DIR}")
  endforeach()
endif()


ament_package()
