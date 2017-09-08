/*
http://www.apache.org/licenses/LICENSE-2.0.txt
Copyright 2016 Intel Corporation
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <snap/config.h>
#include <snap/plugin.h>
#include <snap/proxy/processor_proxy.h>
#include "gmock/gmock.h"

#include <sstream>
#include <string>
#include <vector>

#include "mocks.h"

using Plugin::Config;
using Plugin::ConfigPolicy;
using Plugin::Metric;
using Plugin::StringRule;
using Plugin::Proxy::ProcessorImpl;
using rpc::ConfigMap;
using ::testing::Return;
using ::testing::_;
using ::testing::Invoke;
using std::vector;

string extract_ns(const Metric& metric);

TEST(ProcessorProxySuccessTest, GetConfigPolicyWorks) {
    MockProcessor mockee;
    ON_CALL(mockee, get_config_policy())
            .WillByDefault(Return(mockee.fake_policy));
    rpc::GetConfigPolicyReply resp;
    grpc::Status status;
    EXPECT_NO_THROW({
        ProcessorImpl<> processor(&mockee);
        status = processor.GetConfigPolicy(nullptr, nullptr, &resp);
    });
    EXPECT_EQ(grpc::StatusCode::OK, status.error_code());
    EXPECT_EQ(1, resp.string_policy_size());
}

TEST(ProcessorProxySuccessTest, ProcessWorks) {
    MockProcessor mockee;
    vector<Metric> report;
    auto reporter = [&] (vector<Metric> &metrics, const ::testing::Unused &u2) {
        for (int i =0; i < metrics.size(); i++) {report.emplace_back(metrics.at(i)); }
    };
    rpc::MetricsReply resp;
    grpc::Status status;
    ON_CALL(mockee, process_metrics(_, _))
            .WillByDefault(Invoke(reporter));
    EXPECT_NO_THROW({
        ProcessorImpl<> processor(&mockee);
        rpc::PubProcArg args;
        const string data = "hop";
        mockee.fake_metric.set_data(data);
        *args.add_metrics() = *mockee.fake_metric.get_rpc_metric_ptr();
        status = processor.Process(nullptr, &args, &resp);
    });
    EXPECT_EQ(grpc::StatusCode::OK, status.error_code());
    EXPECT_EQ(1, report.size());
    EXPECT_EQ("hop", report.begin()->get_string_data());
    std::string ns_str = extract_ns(*report.begin());
    EXPECT_EQ("/foo/bar", ns_str);
}

TEST(ProcessorProxySuccessTest, PingWorks) {
    MockProcessor mockee;
    rpc::ErrReply resp;
    grpc::Status status;
    EXPECT_NO_THROW({
                        ProcessorImpl<> processor(&mockee);
                        status = processor.Ping(nullptr, nullptr, &resp);
                    });
    EXPECT_EQ(grpc::StatusCode::OK, status.error_code());
}

TEST(ProcessorProxySuccessTest, KillWorks) {
    MockProcessor mockee;
    rpc::ErrReply resp;
    grpc::Status status;
    EXPECT_NO_THROW({
                        ProcessorImpl<> processor(&mockee);
                        status = processor.Kill(nullptr, nullptr, &resp);
                    });
    EXPECT_EQ(grpc::StatusCode::OK, status.error_code());
}

TEST(ProcessorProxyFailureTest, GetConfigPolicyReportsError) {
    MockProcessor mockee;
    ON_CALL(mockee, get_config_policy())
            .WillByDefault(testing::Throw(Plugin::PluginException("nothing to look at")));
    rpc::GetConfigPolicyReply resp;
    grpc::Status status;
    EXPECT_NO_THROW({
                        ProcessorImpl<> processor(&mockee);
                        status = processor.GetConfigPolicy(nullptr, nullptr, &resp);
                    });
    EXPECT_EQ(grpc::StatusCode::UNKNOWN, status.error_code());
    EXPECT_EQ("nothing to look at", status.error_message());
}

TEST(ProcessorProxySuccessTest, ProcessReportsError) {
    MockProcessor mockee;
    rpc::MetricsReply resp;
    grpc::Status status;
    ON_CALL(mockee, process_metrics(_, _))
            .WillByDefault(testing::Throw(Plugin::PluginException("nothing to look at")));
    EXPECT_NO_THROW({
                        ProcessorImpl<> processor(&mockee);
                        rpc::PubProcArg args;
                        const string data = "hop";
                        mockee.fake_metric.set_data(data);
                        *args.add_metrics() = *mockee.fake_metric.get_rpc_metric_ptr();
                        status = processor.Process(nullptr, &args, &resp);
                    });
    EXPECT_EQ(grpc::StatusCode::UNKNOWN, status.error_code());
    EXPECT_EQ("nothing to look at", status.error_message());
}
