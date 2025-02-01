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


#include "WsEndpoint.hpp"
#include "WsConnection.hpp"

namespace subttxrend
{
namespace common
{

class WsConnection::Impl final
{
  public:
    Impl(std::string const& _name, WsEndpoint& _endpoint)
    {
    }

    ~Impl()
    {
    }

  public:
    void setListener(WsConnectionListener*)
    {
    }

    bool open(std::string const&)
    {
        return true;
    }

    bool close()
    {
        return true;
    }

    bool send(std::string const& message)
    {
        return true;
    }
};


WsConnection::WsConnection(std::string const& name, WsEndpoint& endpoint)
    : impl{std::make_unique<Impl>(name, endpoint)}
{
}
WsConnection::~WsConnection() = default;

void WsConnection::setListener(WsConnectionListener* lstnr)
{
    impl->setListener(lstnr);
}
bool WsConnection::open(std::string const& url)
{
    return impl->open(url);
}
bool WsConnection::close()
{
    return impl->close();
}
bool WsConnection::send(std::string const& message)
{
    return impl->send(message);
}


} // namespace common
} // namespace subttxrend
