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


#include "TtmlAsClient.hpp"

namespace subttxrend
{
namespace common
{

class TtmlAsClient::Impl final
{
  public:
    Impl()
    {
    }

    ~Impl()
    {
    }

    AsHelpers getHelpers()
    {
        return AsHelpers{};
    }

    Properties getData(std::chrono::milliseconds timeout)
    {
        return Properties{};
    }
};


TtmlAsClient::TtmlAsClient()
    :impl{std::make_unique<Impl>()}
{
}
TtmlAsClient::~TtmlAsClient() = default;

Properties TtmlAsClient::getData(std::chrono::milliseconds timeout)
{
    return impl->getData(timeout);
}
AsHelpers TtmlAsClient::getHelpers()
{
    return impl->getHelpers();
}

} // namespace common
} // namespace subttxrend
