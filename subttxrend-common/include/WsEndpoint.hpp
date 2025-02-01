/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 Liberty Global Service B.V.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************/


#pragma once

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <string>
#include <memory>
#include <functional>

namespace subttxrend
{
namespace common
{

using WsConnHdl = websocketpp::connection_hdl;
using WsMessagePtr = websocketpp::config::asio_client::message_type::ptr;

using WsOpenCb = std::function<void(WsConnHdl)>;
using WsCloseCb = std::function<void(WsConnHdl)>;
using WsFailCb = std::function<void(WsConnHdl)>;
using WsMessageCb = std::function<void(WsConnHdl, WsMessagePtr)>;

struct WsConnConfig
{
    WsOpenCb openCb;
    WsCloseCb closeCb;
    WsFailCb failCb;
    WsMessageCb msgCb;
};

class WsEndpoint final
{
  public:
    WsEndpoint();
    WsEndpoint(WsEndpoint&&);
    ~WsEndpoint();

    WsConnHdl connect(std::string const& url, WsConnConfig connCfg);
    bool close(WsConnHdl hdl);
    bool send(WsConnHdl hdl, std::string const& message);

  private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace common
} // namespace subttxrend
