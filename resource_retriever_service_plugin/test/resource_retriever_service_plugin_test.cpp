// Copyright 2026 Open Source Robotics Foundation, Inc.
//
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

#include "resource_retriever_service_plugin/resource_retriever_service_plugin.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <rcl_interfaces/msg/set_parameters_result.hpp>
#include <rclcpp/executors/single_threaded_executor.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/node_options.hpp>
#include <rclcpp/parameter.hpp>
#include <rclcpp/parameter_value.hpp>
#include <rclcpp/service.hpp>
#include <rclcpp/utilities.hpp>
#include <resource_retriever/resource.hpp>
#include <resource_retriever_interfaces/srv/get_resource.hpp>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace resource_retriever_service_plugin
{
namespace
{

using ::resource_retriever_interfaces::srv::GetResource;
using ::testing::_;
using ::testing::Invoke;
using ::testing::MockFunction;
using ::testing::Return;

TEST(RosServiceResourceRetriever, GoodConstruction)
{
  auto node = rclcpp::Node::make_shared("test_message_passing");
  RosServiceResourceRetriever retriever(*node);
}

TEST(RosServiceResourceRetriever, CanHandleUri)
{
  auto node = rclcpp::Node::make_shared("test_get_shared");
  RosServiceResourceRetriever retriever(*node);

  EXPECT_FALSE(retriever.can_handle("something"));
  EXPECT_FALSE(retriever.can_handle("http://something"));
  EXPECT_FALSE(retriever.can_handle("file://something"));
  EXPECT_FALSE(retriever.can_handle("package://something"));
  EXPECT_FALSE(retriever.can_handle("service"));
  EXPECT_FALSE(retriever.can_handle("service://"));

  EXPECT_FALSE(retriever.can_handle("service://:"));
  EXPECT_FALSE(retriever.can_handle("service://:b"));
  EXPECT_FALSE(retriever.can_handle("service://a:b:c"));

  EXPECT_TRUE(retriever.can_handle("service://a:b"));
  EXPECT_TRUE(retriever.can_handle("service://a:"));
}

TEST(RosServiceResourceRetriever, BadGetSharedCall)
{
  auto node = rclcpp::Node::make_shared("test_get_shared");
  RosServiceResourceRetriever retriever(*node);

  EXPECT_EQ(nullptr, retriever.get_shared("something"));
  EXPECT_EQ(nullptr, retriever.get_shared("http://something"));
  EXPECT_EQ(nullptr, retriever.get_shared("file://something"));
  EXPECT_EQ(nullptr, retriever.get_shared("package://something"));
  EXPECT_EQ(nullptr, retriever.get_shared("service"));
  EXPECT_EQ(nullptr, retriever.get_shared("service://"));

  EXPECT_EQ(nullptr, retriever.get_shared("service://:"));
  EXPECT_EQ(nullptr, retriever.get_shared("service://:b"));
  EXPECT_EQ(nullptr, retriever.get_shared("service://a:b:c"));
}

class RosServiceResourceRetrieverTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    executor_ = std::make_unique<rclcpp::executors::SingleThreadedExecutor>();

    client_node_ = rclcpp::Node::make_shared("test_client_node");
    server_node_ = rclcpp::Node::make_shared("test_server_node");
    executor_->add_node(client_node_);
    executor_->add_node(server_node_);

    executor_thread_ = std::make_unique<std::thread>(
      [executor = executor_.get()]() {executor->spin();});

    service_ = server_node_->create_service<GetResource>(
      "test_service",
      [&](const std::shared_ptr<GetResource::Request> request,
      std::shared_ptr<GetResource::Response> response) {
        *response = mock_service_function_.AsStdFunction()(request->path,
        request->etag);
      });

    retriever_ = std::make_unique<RosServiceResourceRetriever>(*client_node_);
  }

  void TearDown() override
  {
    executor_->cancel();
    executor_thread_->join();

    retriever_.reset();
    service_.reset();
    server_node_.reset();
    client_node_.reset();
    executor_thread_.reset();
    executor_.reset();
  }

  std::unique_ptr<rclcpp::executors::SingleThreadedExecutor> executor_;
  std::unique_ptr<std::thread> executor_thread_;
  std::shared_ptr<rclcpp::Node> client_node_;
  std::shared_ptr<rclcpp::Node> server_node_;
  rclcpp::Service<GetResource>::SharedPtr service_;
  std::unique_ptr<RosServiceResourceRetriever> retriever_;

  MockFunction<GetResource::Response(const std::string &, const std::string &)>
  mock_service_function_;
};

TEST_F(RosServiceResourceRetrieverTest, SimpleE2EGet)
{
  std::vector<uint8_t> resource_data(2u, 3);
  GetResource::Response response;
  response.status_code = GetResource::Response::OK;
  response.body = resource_data;

  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(Return(response));

  auto resource = retriever_->get_shared("service://test_service:a");
  ASSERT_NE(nullptr, resource);
  EXPECT_EQ(resource->data, resource_data);
}

TEST_F(RosServiceResourceRetrieverTest, ResourcePathExtraction)
{
  std::vector<uint8_t> resource_data(3u, 9);
  GetResource::Response response;
  response.status_code = GetResource::Response::OK;
  response.body = resource_data;

  // Expect the service to be called with the exact resource path after the ':'
  EXPECT_CALL(mock_service_function_, Call(std::string("foo/bar"), _)).WillOnce(Return(response));

  auto resource = retriever_->get_shared("service://test_service:foo/bar");
  ASSERT_NE(nullptr, resource);
  EXPECT_EQ(resource->data, resource_data);
}

TEST_F(RosServiceResourceRetrieverTest, ErrorStatusReturnsNull)
{
  GetResource::Response response;
  response.status_code = GetResource::Response::ERROR;

  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(Return(response));

  auto resource = retriever_->get_shared("service://test_service:a");
  EXPECT_EQ(resource, nullptr);
}

TEST_F(RosServiceResourceRetrieverTest, UnknownStatusReturnsNull)
{
  GetResource::Response response;
  response.status_code = 17;

  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(Return(response));

  auto resource = retriever_->get_shared("service://test_service:a");
  EXPECT_EQ(resource, nullptr);
}

TEST_F(RosServiceResourceRetrieverTest, DuplicateCallsWithoutEtagReturnCachedValue)
{
  std::vector<uint8_t> resource_data(2u, 3);
  GetResource::Response response1;
  response1.status_code = GetResource::Response::OK;
  response1.body = resource_data;
  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(Return(response1));

  auto resource1 = retriever_->get_shared("service://test_service:a");
  ASSERT_NE(nullptr, resource1);
  EXPECT_EQ(resource1->data, resource_data);

  auto resource2 = retriever_->get_shared("service://test_service:a");
  ASSERT_NE(nullptr, resource2);
  EXPECT_EQ(resource2->data, resource_data);

  EXPECT_EQ(resource1, resource2);
}

TEST_F(RosServiceResourceRetrieverTest, NotModifiedStatusReturnsCachedValue)
{
  std::vector<uint8_t> resource_data(2u, 3);
  GetResource::Response response1;
  response1.status_code = GetResource::Response::OK;
  response1.body = resource_data;
  response1.etag = "something";
  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(Return(response1));

  auto resource1 = retriever_->get_shared("service://test_service:a");
  ASSERT_NE(nullptr, resource1);
  EXPECT_EQ(resource1->data, resource_data);

  GetResource::Response response2;
  response2.status_code = GetResource::Response::NOT_MODIFIED;
  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(Return(response2));

  auto resource2 = retriever_->get_shared("service://test_service:a");
  ASSERT_NE(nullptr, resource2);
  EXPECT_EQ(resource2->data, resource_data);

  EXPECT_EQ(resource1, resource2);
}

TEST_F(RosServiceResourceRetrieverTest, NotModifiedReturnsEmptyCachedValue)
{
  GetResource::Response response;
  response.status_code = GetResource::Response::NOT_MODIFIED;
  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(Return(response));

  auto resource = retriever_->get_shared("service://test_service:a");
  EXPECT_EQ(resource, nullptr);
}

TEST_F(RosServiceResourceRetrieverTest, MultipleServices)
{
  MockFunction<GetResource::Response(const std::string &, const std::string &)>
  mock_service_function2;
  auto service2 = server_node_->create_service<GetResource>(
    "test_service2",
    [&](const std::shared_ptr<GetResource::Request> request,
    std::shared_ptr<GetResource::Response> response) {
      *response = mock_service_function2.AsStdFunction()(request->path,
      request->etag);
    });

  std::vector<uint8_t> resource_data1(2u, 3);
  GetResource::Response response1;
  response1.status_code = GetResource::Response::OK;
  response1.body = resource_data1;

  std::vector<uint8_t> resource_data2(2u, 8);
  GetResource::Response response2;
  response2.status_code = GetResource::Response::OK;
  response2.body = resource_data2;

  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(Return(response1));

  auto resource1 = retriever_->get_shared("service://test_service:a");
  ASSERT_NE(nullptr, resource1);
  EXPECT_EQ(resource1->data, resource_data1);

  EXPECT_CALL(mock_service_function2, Call(_, _)).WillOnce(Return(response2));

  auto resource2 = retriever_->get_shared("service://test_service2:a");
  ASSERT_NE(nullptr, resource2);
  EXPECT_EQ(resource2->data, resource_data2);
}

TEST_F(RosServiceResourceRetrieverTest, ServiceTimeoutViaParameterOverrideReturnsNull)
{
  rclcpp::NodeOptions options;
  options.append_parameter_override(
    std::string(RosServiceResourceRetriever::service_timeout_param_name), 50);
  auto custom_node = rclcpp::Node::make_shared("test_override_client_node", options);
  RosServiceResourceRetriever custom_retriever(*custom_node);

  GetResource::Response response;
  response.status_code = GetResource::Response::OK;
  response.body = std::vector<uint8_t>(2u, 3);

  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(
    Invoke(
      [response](const std::string &, const std::string &) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return response;
      }));

  auto resource = custom_retriever.get_shared("service://test_service:a");
  EXPECT_EQ(resource, nullptr);
}

TEST_F(RosServiceResourceRetrieverTest, RuntimeServiceTimeoutParameterUpdate)
{
  const std::string param_name(RosServiceResourceRetriever::service_timeout_param_name);

  // Set runtime timeout to 50ms -> 200ms service call should time out.
  auto set_result = client_node_->set_parameter(rclcpp::Parameter(param_name, 50));
  ASSERT_TRUE(set_result.successful);

  std::vector<uint8_t> resource_data(2u, 3);
  GetResource::Response response;
  response.status_code = GetResource::Response::OK;
  response.body = resource_data;

  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(
    Invoke(
      [response](const std::string &, const std::string &) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return response;
      }));

  EXPECT_EQ(nullptr, retriever_->get_shared("service://test_service:a"));

  // Update runtime timeout to 1000ms -> 100ms service call should now succeed.
  set_result = client_node_->set_parameter(rclcpp::Parameter(param_name, 1000));
  ASSERT_TRUE(set_result.successful);

  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(
    Invoke(
      [response](const std::string &, const std::string &) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return response;
      }));

  auto resource = retriever_->get_shared("service://test_service:a");
  ASSERT_NE(nullptr, resource);
  EXPECT_EQ(resource->data, resource_data);
}

TEST_F(RosServiceResourceRetrieverTest, InvalidRuntimeServiceTimeoutParameterRejected)
{
  const std::string param_name(RosServiceResourceRetriever::service_timeout_param_name);

  ASSERT_TRUE(client_node_->set_parameter(rclcpp::Parameter(param_name, 500)).successful);

  EXPECT_FALSE(client_node_->set_parameter(rclcpp::Parameter(param_name, 0)).successful);
  EXPECT_FALSE(client_node_->set_parameter(rclcpp::Parameter(param_name, -50)).successful);
  EXPECT_FALSE(
    client_node_->set_parameter(
      rclcpp::Parameter(param_name, std::numeric_limits<int64_t>::max())).successful);
  EXPECT_FALSE(
    client_node_->set_parameter(
      rclcpp::Parameter(
        param_name,
        static_cast<int64_t>(RosServiceResourceRetriever::max_service_timeout.count() + 1))).
    successful);
  EXPECT_FALSE(
    client_node_->set_parameter(rclcpp::Parameter(param_name, "invalid_timeout")).successful);

  // Previous valid timeout (500ms) remains intact.
  EXPECT_EQ(500, client_node_->get_parameter(param_name).as_int());
}

TEST_F(RosServiceResourceRetrieverTest, InvalidInitialServiceTimeoutParameterFallsBackToDefault)
{
  const std::string param_name(RosServiceResourceRetriever::service_timeout_param_name);
  const std::vector<rclcpp::ParameterValue> invalid_overrides = {
    rclcpp::ParameterValue(0),
    rclcpp::ParameterValue(-50),
    rclcpp::ParameterValue(std::numeric_limits<int64_t>::max()),
    rclcpp::ParameterValue(
      static_cast<int64_t>(RosServiceResourceRetriever::max_service_timeout.count() + 1)),
    rclcpp::ParameterValue(std::string("invalid_timeout")),
  };

  std::vector<uint8_t> resource_data(2u, 3);
  GetResource::Response response;
  response.status_code = GetResource::Response::OK;
  response.body = resource_data;

  for (size_t i = 0; i < invalid_overrides.size(); ++i) {
    rclcpp::NodeOptions options;
    options.append_parameter_override(param_name, invalid_overrides[i]);
    auto custom_node = rclcpp::Node::make_shared(
      "test_invalid_override_node_" + std::to_string(i), options);
    RosServiceResourceRetriever custom_retriever(*custom_node);

    EXPECT_EQ(
      RosServiceResourceRetriever::default_service_timeout.count(),
      custom_node->get_parameter(param_name).as_int());

    EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(
      Invoke(
        [response](const std::string &, const std::string &) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          return response;
        }));

    auto resource = custom_retriever.get_shared("service://test_service:a");
    ASSERT_NE(nullptr, resource);
    EXPECT_EQ(resource->data, resource_data);
  }
}

TEST_F(RosServiceResourceRetrieverTest, RetrieversOnSameNodeShareServiceTimeoutParameter)
{
  const std::string param_name(RosServiceResourceRetriever::service_timeout_param_name);
  rclcpp::NodeOptions options;
  options.append_parameter_override(param_name, 50);
  auto custom_node = rclcpp::Node::make_shared("test_shared_timeout_node", options);

  // The second retriever finds the parameter already declared by the first one.
  RosServiceResourceRetriever first_retriever(*custom_node);
  RosServiceResourceRetriever second_retriever(*custom_node);
  EXPECT_EQ(50, custom_node->get_parameter(param_name).as_int());

  std::vector<uint8_t> resource_data(2u, 3);
  GetResource::Response response;
  response.status_code = GetResource::Response::OK;
  response.body = resource_data;

  auto service_delay_ms = std::make_shared<std::atomic<int>>(200);
  EXPECT_CALL(mock_service_function_, Call(_, _)).WillRepeatedly(
    Invoke(
      [response, service_delay_ms](const std::string &, const std::string &) {
        std::this_thread::sleep_for(std::chrono::milliseconds(service_delay_ms->load()));
        return response;
      }));

  // 50ms timeout from the override -> 200ms service calls time out for both retrievers.
  EXPECT_EQ(nullptr, first_retriever.get_shared("service://test_service:a"));
  EXPECT_EQ(nullptr, second_retriever.get_shared("service://test_service:b"));

  // A runtime update applies to both retrievers.
  ASSERT_TRUE(custom_node->set_parameter(rclcpp::Parameter(param_name, 2000)).successful);
  service_delay_ms->store(10);

  auto first_resource = first_retriever.get_shared("service://test_service:c");
  ASSERT_NE(nullptr, first_resource);
  EXPECT_EQ(first_resource->data, resource_data);

  auto second_resource = second_retriever.get_shared("service://test_service:d");
  ASSERT_NE(nullptr, second_resource);
  EXPECT_EQ(second_resource->data, resource_data);
}

TEST(RosServiceResourceRetriever, ServiceTimeoutParameterRangeOutlivesRetriever)
{
  const std::string param_name(RosServiceResourceRetriever::service_timeout_param_name);
  const int64_t max_timeout_ms = RosServiceResourceRetriever::max_service_timeout.count();
  auto node = rclcpp::Node::make_shared("test_timeout_range_node");
  {
    RosServiceResourceRetriever retriever(*node);
  }

  // The range belongs to the parameter, so it is still enforced without any retriever alive.
  const auto descriptor = node->describe_parameter(param_name);
  ASSERT_EQ(1u, descriptor.integer_range.size());
  EXPECT_EQ(1, descriptor.integer_range[0].from_value);
  EXPECT_EQ(max_timeout_ms, descriptor.integer_range[0].to_value);

  EXPECT_FALSE(node->set_parameter(rclcpp::Parameter(param_name, 0)).successful);
  EXPECT_FALSE(node->set_parameter(rclcpp::Parameter(param_name, -50)).successful);
  EXPECT_FALSE(
    node->set_parameter(rclcpp::Parameter(param_name, max_timeout_ms + 1)).successful);
  EXPECT_TRUE(node->set_parameter(rclcpp::Parameter(param_name, max_timeout_ms)).successful);

  // A retriever created afterwards reuses the existing parameter.
  RosServiceResourceRetriever retriever(*node);
  EXPECT_EQ(max_timeout_ms, node->get_parameter(param_name).as_int());
}

TEST_F(RosServiceResourceRetrieverTest, RejectedAtomicParameterUpdateKeepsServiceTimeout)
{
  const std::string param_name(RosServiceResourceRetriever::service_timeout_param_name);
  auto custom_node = rclcpp::Node::make_shared("test_atomic_update_node");

  // Another part of the node validates an unrelated parameter.
  auto callback_handle = custom_node->add_on_set_parameters_callback(
    [](const std::vector<rclcpp::Parameter> & parameters) {
      rcl_interfaces::msg::SetParametersResult result;
      result.successful = true;
      for (const auto & parameter : parameters) {
        if (parameter.get_name() == "other_parameter" && parameter.as_int() < 0) {
          result.successful = false;
          result.reason = "other_parameter must not be negative";
        }
      }
      return result;
    });
  custom_node->declare_parameter("other_parameter", 0);
  RosServiceResourceRetriever custom_retriever(*custom_node);

  // The update is rejected as a whole, so the 1ms timeout must not be applied.
  const auto set_result = custom_node->set_parameters_atomically(
    {rclcpp::Parameter(param_name, 1), rclcpp::Parameter("other_parameter", -1)});
  EXPECT_FALSE(set_result.successful);
  EXPECT_EQ(
    RosServiceResourceRetriever::default_service_timeout.count(),
    custom_node->get_parameter(param_name).as_int());

  std::vector<uint8_t> resource_data(2u, 3);
  GetResource::Response response;
  response.status_code = GetResource::Response::OK;
  response.body = resource_data;

  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(
    Invoke(
      [response](const std::string &, const std::string &) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return response;
      }));

  auto resource = custom_retriever.get_shared("service://test_service:a");
  ASSERT_NE(nullptr, resource);
  EXPECT_EQ(resource->data, resource_data);
}

TEST_F(RosServiceResourceRetrieverTest, ServiceTimeoutParameterDeclaredByNodeIsHonored)
{
  const std::string param_name(RosServiceResourceRetriever::service_timeout_param_name);
  auto custom_node = rclcpp::Node::make_shared("test_node_declared_timeout_node");

  // The node declares the parameter itself, so the retriever must not declare it again.
  custom_node->declare_parameter(param_name, 50);
  RosServiceResourceRetriever custom_retriever(*custom_node);

  GetResource::Response response;
  response.status_code = GetResource::Response::OK;
  response.body = std::vector<uint8_t>(2u, 3);

  EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(
    Invoke(
      [response](const std::string &, const std::string &) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return response;
      }));

  EXPECT_EQ(nullptr, custom_retriever.get_shared("service://test_service:a"));
}

TEST_F(
  RosServiceResourceRetrieverTest,
  InvalidServiceTimeoutParameterDeclaredByNodeFallsBackToDefault)
{
  const std::string param_name(RosServiceResourceRetriever::service_timeout_param_name);
  // Declared by the node without a range, so nothing prevents these unusable values.
  const std::vector<rclcpp::ParameterValue> invalid_values = {
    rclcpp::ParameterValue(0),
    rclcpp::ParameterValue(1.5),
  };

  std::vector<uint8_t> resource_data(2u, 3);
  GetResource::Response response;
  response.status_code = GetResource::Response::OK;
  response.body = resource_data;

  for (size_t i = 0; i < invalid_values.size(); ++i) {
    auto custom_node = rclcpp::Node::make_shared(
      "test_node_declared_invalid_timeout_node_" + std::to_string(i));
    custom_node->declare_parameter(param_name, invalid_values[i]);
    RosServiceResourceRetriever custom_retriever(*custom_node);

    // A 0ms timeout would make the 100ms service call fail.
    EXPECT_CALL(mock_service_function_, Call(_, _)).WillOnce(
      Invoke(
        [response](const std::string &, const std::string &) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          return response;
        }));

    auto resource = custom_retriever.get_shared("service://test_service:a");
    ASSERT_NE(nullptr, resource);
    EXPECT_EQ(resource->data, resource_data);
  }
}

}  // namespace
}  // namespace resource_retriever_service_plugin

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  rclcpp::init(argc, argv);
  const int result = RUN_ALL_TESTS();
  rclcpp::shutdown();
  return result;
}
