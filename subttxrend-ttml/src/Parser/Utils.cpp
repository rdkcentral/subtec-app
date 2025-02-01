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


#include "Utils.hpp"

#include <algorithm>
#include <limits>

namespace subttxrend
{
namespace ttmlengine
{

bool isSpace(int c)
{
    static constexpr int SAFE_CHAR_MIN = std::numeric_limits<unsigned char>::min();
    static constexpr int SAFE_CHAR_MAX = std::numeric_limits<unsigned char>::max();

    if ((c >= SAFE_CHAR_MIN) && (c <= SAFE_CHAR_MAX))
    {
        return std::isspace(c);
    }
    else
    {
        return false;
    }
}

std::string trimWhitespace(const std::string &s)
{
    auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) {return isSpace(c);});
    auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c) {return isSpace(c);}).base();

    return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
}

} // namespace subttxrend
} // namespace ttmlengine
