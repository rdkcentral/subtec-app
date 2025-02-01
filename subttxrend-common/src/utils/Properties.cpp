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


#include "Properties.hpp"

namespace subttxrend
{
namespace common
{

std::string Properties::toString() const
{
    std::string ret;
    for(auto const& prop : properties) {
        ret.append(prop.first).append(": ").append(prop.second).append(", ");
    }
    return ret;
}

void Properties::clear()
{
    properties.clear();
}

bool Properties::hasKey(std::string const& k) const
{
    return find(k) != end();
}

std::string const& Properties::getString(std::string const& k) const
{
    auto it = find(k);
    if (it != end()) {
        return getString(it);
    }
    throw std::logic_error("No value for key: " + k);
}

void Properties::setString(std::string const& key, std::string const& value)
{
    properties[key] = value;
}


Properties::ConstIterator Properties::end() const
{
    return properties.end();
}
Properties::ConstIterator Properties::find(std::string const& k) const
{
    return properties.find(k);
}
std::string const& Properties::getString(ConstIterator it) const
{
    return it->second;
}


Properties& paramsToProperties(Properties& p, std::string const& params)
{
    auto kv = [&p](std::string s) {
        if (s.empty())
            return;
        auto k_v = [&p, cnt = 0, k = std::string()](std::string s) mutable {
            if (cnt == 0) {
                k = s;
            } else {
                p.setString(k, s);
            }
            cnt++;
        };
        split(s, ':', k_v, 1);
    };
    split(params, ';', kv);
    return p;
}


std::ostream& operator<<(std::ostream& os, Properties const& props)
{
    return os << props.toString();
}

} // namespace common
} // namespace subttxrend

