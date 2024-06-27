// Copyright 2022-2024 Fraunhofer Italia Research

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Copyright 2022 Fraunhofer Italia Research. All Rights Reserved.
// Author: Michael Terzer (michael.terzer@fraunhofer.it)

#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_srvs/srv/trigger.hpp"

using ServiceT = std_srvs::srv::Trigger;

int number{0};

void callback(
  const std::shared_ptr<ServiceT::Request> request, std::shared_ptr<ServiceT::Response> response)
{
  response->success = true;
  response->message = "hello bt service client! " + std::to_string(number);
  number++;
}

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto nh = std::make_shared<rclcpp::Node>("trigger_service_server");
  rclcpp::Service<ServiceT>::SharedPtr service =
    nh->create_service<ServiceT>("test_service_server", &callback);

  rclcpp::spin(nh);
  rclcpp::shutdown();
}