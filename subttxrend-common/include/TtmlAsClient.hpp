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

#include "AsListener.hpp"
#include "Properties.hpp"
#include <memory>


namespace subttxrend
{
namespace common
{

enum class TtmlProperty
{
    subtitles_style_rules, // set of rules implying style for particular subtitles, eg {"eng", "hh", "outline"}
    max,
};

std::ostream& operator<<(std::ostream& out, TtmlProperty prop);

class TtmlAsClient final : public AsListener
{
  public:
    TtmlAsClient();
    ~TtmlAsClient();

    AsHelpers getHelpers() override;
    Properties getData();

  private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace common
} // namespace subttxrend
