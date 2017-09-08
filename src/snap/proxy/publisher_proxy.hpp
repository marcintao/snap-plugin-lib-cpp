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
#pragma once

#include<vector>

#include <grpc++/grpc++.h>

#include "snap/rpc/plugin.pb.h"

using google::protobuf::RepeatedPtrField;

using grpc::Server;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

using rpc::Empty;
using rpc::ErrReply;
using rpc::GetConfigPolicyReply;
using rpc::KillArg;
using rpc::Publisher;
using rpc::PubProcArg;

using Plugin::Proxy::PublisherImpl;

template <class Base, class Context>
PublisherImpl<Base,Context>::PublisherImpl(Plugin::PublisherInterface* plugin) :
                             publisher(plugin) {
    plugin_impl_ptr = new PluginImpl<Context>(plugin);
}

template <class Base, class Context>
PublisherImpl<Base,Context>::~PublisherImpl() {
    delete plugin_impl_ptr;
}

template <class Base, class Context>
Status PublisherImpl<Base,Context>::Publish(Context* context, const PubProcArg* req,
                              ErrReply* resp) {
    std::vector<Metric> metrics;
    RepeatedPtrField<rpc::Metric> rpc_mets = req->metrics();

    for (int i = 0; i < rpc_mets.size(); i++) {
        metrics.emplace_back(rpc_mets.Mutable(i));
    }

    Plugin::Config config(const_cast<rpc::ConfigMap&>(req->config()));
    try {
        publisher->publish_metrics(metrics, config);
        return Status::OK;
    } catch (PluginException &e) {
        resp->set_error(e.what());
        return Status(StatusCode::UNKNOWN, e.what());
    }
}

template <class Base, class Context>
Status PublisherImpl<Base,Context>::Kill(Context* context, const KillArg* req,
                           ErrReply* resp) {
    return plugin_impl_ptr->Kill(context, req, resp);
}

template <class Base, class Context>
Status PublisherImpl<Base,Context>::GetConfigPolicy(Context* context, const Empty* req,
                                      GetConfigPolicyReply* resp) {
    try {
        return plugin_impl_ptr->GetConfigPolicy(context, req, resp);
    } catch (PluginException &e) {
        resp->set_error(e.what());
        return Status(StatusCode::UNKNOWN, e.what());
    }
}

template <class Base, class Context>
Status PublisherImpl<Base,Context>::Ping(Context* context, const Empty* req,
                           ErrReply* resp) {
    return plugin_impl_ptr->Ping(context, req, resp);
}
