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

#include <grpc++/grpc++.h>

#include "snap/rpc/plugin.pb.h"
#include "snap/rpc/plugin.grpc.pb.h"

#include "snap/proxy/plugin_proxy.h"

namespace Plugin {
    namespace Proxy {
        template <class Base=rpc::Publisher::Service, class Context=grpc::ServerContext>
        class PublisherImpl final : public Base {
        public:
            explicit PublisherImpl(Plugin::PublisherInterface* plugin);

            ~PublisherImpl();

            grpc::Status Publish(Context* context, const rpc::PubProcArg* req,
                                rpc::ErrReply* resp);

            grpc::Status Kill(Context* context, const rpc::KillArg* request,
                                rpc::ErrReply* response);

            grpc::Status GetConfigPolicy(Context* context,
                                        const rpc::Empty* request,
                                        rpc::GetConfigPolicyReply* resp);

            grpc::Status Ping(Context* context, const rpc::Empty* request,
                                rpc::ErrReply* resp);

        private:
            Plugin::PublisherInterface* publisher;
            PluginImpl<Context>* plugin_impl_ptr;
        };
    }  // namespace Proxy
}  // namespace Plugin

#include "snap/proxy/publisher_proxy.hpp"
