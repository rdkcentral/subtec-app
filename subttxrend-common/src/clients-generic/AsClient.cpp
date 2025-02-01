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


#include "AsClient.hpp"
#include "WsConnection.hpp"
#include "AsListener.hpp"

namespace subttxrend
{
namespace common
{

class AsClient::Impl final : private AsSendHelper, private WsConnectionListener
{
  public:
    Impl(std::chrono::milliseconds,
        std::unique_ptr<WsConnection>)
    {
    }

    ~Impl()
    {
    }

    void setup(AsHelpers)
    {
    }

    bool send(std::string const& message) override
    {
        return true;
    }

    void onConnOpen() override
    {
    }

    void onConnClose() override
    {
    }

    void onConnFail() override
    {
    }

    void onConnMessage(std::string const& msg) override
    {
    }

    AsSendHelper& getSendHelper()
    {
        return *this;
    }
};


AsClient::AsClient(std::chrono::milliseconds connStatusCheckTimeout,
    std::unique_ptr<WsConnection> conn)
    : impl{std::make_unique<Impl>(connStatusCheckTimeout, std::move(conn))}
{
}

AsClient::~AsClient() = default;

void AsClient::setup(AsHelpers helpers)
{
    impl->setup(std::move(helpers));
}

AsSendHelper& AsClient::getSendHelper()
{
    return impl->getSendHelper();
}

} // namespace common
} // namespace subttxrend
