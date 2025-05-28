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

#include <string>
#include <vector>
#include <chrono>
#include "Properties.hpp"



namespace subttxrend
{
namespace common
{

class JsonData;

class AsHelper
{
  public:
    virtual ~AsHelper() = default;

    virtual std::string const& getListenedEvent() const = 0;
    virtual std::string const& getDataToAcquire() const = 0;

    // called for data obtained by update EVENT
    virtual void onUpdate(JsonData const& json) = 0;
    // called for data obtained by GET
    virtual void onData(JsonData const& json) = 0;
};

using AsHelpers = std::vector<AsHelper*>;

class AsSendHelper
{
  public:
    virtual ~AsSendHelper() = default;
    virtual bool send(std::string const& msg) = 0;
};

class AsListener
{
  public:
    virtual ~AsListener() = default;
    virtual AsHelpers getHelpers() = 0;
    virtual Properties getData() = 0;
};

} // namespace common
} // namespace subttxrend
