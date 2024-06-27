cmake_minimum_required(VERSION 3.5)
project(bt_executor)

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
find_package(behavior_tree_msgs REQUIRED)
find_package(pluginlib REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(behaviortree_ros2 REQUIRED)
find_package(behavior_tree_msgs REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(TinyXML2 REQUIRED)

if (ZeroMQ_FOUND)
  add_definitions(-DZMQ_FOUND)
endif()

###########
## Build ##
###########

include_directories(include ${YAML_CPP_INCLUDEDIR} ${TinyXML2_INCLUDE_DIRS})

set(LIBRARY_NAME ${PROJECT_NAME})

set(dependencies
  rclcpp 
  behavior_tree_msgs
  pluginlib
  behaviortree_cpp
  behaviortree_ros2
  behavior_tree_msgs
  yaml-cpp
)

add_library(${LIBRARY_NAME} SHARED
  src/bt_executor.cpp 
  src/bt_manager.cpp
  src/skill_library_base.cpp
  src/bt_skill_manager.cpp
  src/descriptor.cpp
)
target_link_libraries(${LIBRARY_NAME} ${YAML_CPP_LIBRARIES})

set(EXECUTABLE_NAME ${PROJECT_NAME}_node)
add_executable(${EXECUTABLE_NAME}
  src/bt_executor_node.cpp
)
ament_target_dependencies(${EXECUTABLE_NAME}
  ${dependencies}
)
target_link_libraries(${EXECUTABLE_NAME} ${LIBRARY_NAME} ${YAML_CPP_LIBRARIES})
ament_target_dependencies(${LIBRARY_NAME}
  ${dependencies}
)

if (ZeroMQ_FOUND)
  target_link_libraries(${EXECUTABLE_NAME} ${LIBRARY_NAME} ${YAML_CPP_LIBRARIES} ${ZeroMQ_LIBRARIES})
endif()

ament_export_libraries(
${LIBRARY_NAME}
)
ament_export_include_directories(
  include
)

ament_export_dependencies(${dependencies})

#############
## Install ##
#############

install(TARGETS ${LIBRARY_NAME}
  EXPORT ${LIBRARY_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(TARGETS ${EXECUTABLE_NAME}
  RUNTIME DESTINATION lib/${LIBRARY_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)

install(DIRECTORY
  config
  launch
  test
  DESTINATION 
  share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  find_package(ament_lint_auto REQUIRED)
  find_package(ros_testing REQUIRED)

  # the following lines skip linters
  set(ament_cmake_cppcheck_FOUND TRUE)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  set(ament_cmake_flake8_FOUND TRUE)
  set(ament_cmake_uncrustify_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()

  # Run all lint tests in package.xml except those listed above
  ament_lint_auto_find_test_dependencies()

  ament_add_gtest_executable(gtest_bt_executor
      test/bt_executor_main_test.cpp
      test/unittest_bt_executor.cpp
  )
  ament_target_dependencies(gtest_bt_executor ${dependencies} ${THIS_PACKAGE_INCLUDE_DEPENDS})
  target_link_libraries(gtest_bt_executor
    ${LIBRARY_NAME}
  ) 
  add_ros_test(test/test_bt_executor.test.py ARGS "test_binary_dir:=${CMAKE_CURRENT_BINARY_DIR}")
endif()


ament_package()