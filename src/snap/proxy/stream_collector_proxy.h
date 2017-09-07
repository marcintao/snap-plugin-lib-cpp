#ifndef STREAM_COLLECTOR_PROXY_H
#define STREAM_COLLECTOR_PROXY_H
/*
http://www.apache.org/licenses/LICENSE-2.0.txt
Copyright 2017 Intel Corporation
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
#include <chrono>
#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "snap/rpc/plugin.grpc.pb.h"
#include "snap/rpc/plugin.pb.h"

#include "snap/proxy/plugin_proxy.h"

namespace Plugin {
    namespace Proxy {
        template <class ServiceBase=rpc::StreamCollector::Service, class ServiceContext=grpc::ServerContext>
        class StreamCollectorImpl final : public ServiceBase {
        public:
            explicit StreamCollectorImpl(Plugin::StreamCollectorInterface* plugin);

            ~StreamCollectorImpl();

            grpc::Status GetMetricTypes(ServiceContext* context,
                                        const rpc::GetMetricTypesArg* request,
                                        rpc::MetricsReply* resp);

            grpc::Status Kill(ServiceContext* context, const rpc::KillArg* request,
                            rpc::ErrReply* response);

            grpc::Status GetConfigPolicy(ServiceContext* context,
                                        const rpc::Empty* request,
                                        rpc::GetConfigPolicyReply* resp);

            grpc::Status Ping(ServiceContext* context, const rpc::Empty* request,
                            rpc::ErrReply* resp);
            
            grpc::Status StreamMetrics(ServiceContext* context,
                                        grpc::ServerReaderWriter<rpc::CollectReply, rpc::CollectArg>* stream);
            grpc::Status streamMetricsInt(ServiceContext* context,
                                        grpc::WriterInterface<rpc::CollectReply>* streamOut, grpc::ReaderInterface<rpc::CollectArg>* streamIn);

            void SetMaxCollectDuration(std::chrono::seconds maxCollectDuration) {
                _max_collect_duration = maxCollectDuration;
            }
            std::chrono::seconds GetMaxCollectDuration () { 
                return _max_collect_duration; 
            }
            void SetMaxMetricsBuffer(int64_t maxMetricsBuffer) {
                _max_metrics_buffer = maxMetricsBuffer;
            }
            int64_t GetMaxMetricsBuffer() { 
                return _max_metrics_buffer; 
            }

            bool errorSend(ServiceContext* context,
                grpc::WriterInterface<rpc::CollectReply>* stream);
            bool metricSend(const std::string &taskID,
                            ServiceContext* context,
                            grpc::WriterInterface<rpc::CollectReply>* stream);
            bool streamRecv(const std::string &taskID,
                            ServiceContext* context,
                            grpc::ReaderInterface<rpc::CollectArg>* stream);
            bool sendReply(const std::string &taskID,
                            grpc::WriterInterface<rpc::CollectReply>* stream);
            bool PutSendMetsAndErrMsg(ServiceContext* context);
            
            void ErrChanClose() {
                _errChan.close();
            }
            bool ErrChanIsClosed() {
                return _errChan.is_closed();
            }
            void ErrChanPut(const std::string &errMsg) {
                _errChan.put(errMsg);
            }
            bool ErrChanGet(std::string &errMsg) {
                return _errChan.get(errMsg);
            }

            void SendChanClose() {
                _sendChan.close();
            }
            bool SendChanIsClosed() {
                return _sendChan.is_closed();
            }
            void SendChanPut(const std::vector<Metric> &metrics) {
                _sendChan.put(metrics);
            }
            bool SendChanGet(std::vector<Metric> &metrics) {
                return _sendChan.get(metrics);
            }

            void RecvChanClose() {
                _recvChan.close();
            }
            bool RecvChanIsClosed() {
                return _recvChan.is_closed();
            }
            void RecvChanPut(const std::vector<Metric> &metrics) {
                _recvChan.put(metrics);
            }
            bool RecvChanGet(std::vector<Metric> &metrics) {
                return _recvChan.get(metrics);
            }

            template <class T>
            class StreamChannel {
            private:
                std::list<T> _queue;
                std::mutex _m;
                std::condition_variable _cv;
                bool _closed;

            public:
                StreamChannel() : _closed(false) {}

                void close();
                bool is_closed();
                void put(const T &in);
                bool get(T &out, bool wait = true);
            };

        private:
            Plugin::StreamCollectorInterface* _stream_collector;
            PluginImpl<ServiceContext>* _plugin_impl_ptr;
            ServiceContext* _ctx;
            int64_t _max_metrics_buffer;
            std::chrono::seconds _max_collect_duration;
            rpc::CollectReply _collect_reply;
            rpc::MetricsReply *_metrics_reply;
            rpc::ErrReply *_err_reply;

            StreamChannel<std::vector<Plugin::Metric>> _sendChan;
            StreamChannel<std::vector<Plugin::Metric>> _recvChan;
            StreamChannel<std::string> _errChan;          
        };
    }  // namespace Proxy
}  // namespace Plugin

#include "snap/proxy/stream_collector_proxy.hpp"

#endif /* STREAM_COLLECTOR_PROXY_H */
