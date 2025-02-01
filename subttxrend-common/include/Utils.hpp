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

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <mutex>
#include <sstream>

// TODO reuse implementation from ipp2::utils
namespace subttxrend
{
namespace common
{
using UniqueLock = std::unique_lock<std::mutex>;
using LockGuard = std::lock_guard<std::mutex>;

using Tokens = std::vector<std::string>;


namespace details {

static inline std::string to_string(std::ostringstream& out)
{
    return out.str();
}

template <class T, class... Tail>
std::string to_string(std::ostringstream& out, T&& v, Tail&&... tail)
{
    out << v;
    return to_string(out, std::forward<Tail>(tail)...);
}

} // namespace details

template <class... Args>
std::string to_string(Args&& ...args)
{
    std::ostringstream out;
    return details::to_string(out, std::forward<Args>(args)...);
}

template<class T>
using is_scoped_enum = std::integral_constant<
    bool,
    std::is_enum<T>::value && !std::is_convertible<T, std::underlying_type_t<T>>::value>;

template <class T>
constexpr std::underlying_type_t<T> enum_to_int(T e)
{
    static_assert(std::is_enum<T>::value, "Given argument type is not enum");
    return static_cast<std::underlying_type_t<T>>(e);
}

template <class T>
T from_string(std::string const& str)
{
    using namespace std::literals::string_literals;

    std::istringstream in{str};
    T value;
    if (in >> value)
        return value;
    throw std::invalid_argument{"Can't create instance of type "s + typeid(T).name() + " from " + str};
}

template <>
bool from_string<bool>(std::string const& str);
template <>
std::string from_string<std::string>(std::string const& str);

size_t getPart(std::string const&, size_t startPosition, char const);

template<typename T, typename... Params>
size_t getPart(std::string const& str, size_t startPosition, char const delimiter, T& part, Params&... parts)
{
    auto separatorPosition = str.find(delimiter, startPosition);
    part = str.substr(startPosition, separatorPosition - startPosition);

    if (separatorPosition != std::string::npos) {
        startPosition = separatorPosition + 1;
        separatorPosition = getPart(str, startPosition, delimiter, parts...);
    }

    return separatorPosition;
}

Tokens tokenize(std::string const& str, char const delimiter);

template <class CharT, class Traits, class F>
size_t split(std::basic_string<CharT, Traits> const& str, CharT del, F fun, size_t max = 0)
{
    using TheString = std::basic_string<CharT, Traits>;
    size_t count{};
    size_t start{};
    size_t stop = str.find(del);
    while (stop != TheString::npos) {
        fun(str.substr(start, stop - start));
        start = stop + 1;
        stop = str.find(del, start);
        count++;
        if ((max > 0) && (count == max)) {
            break;
        }
    }

    fun(str.substr(start));
    count++;
    return count;
}

} // namespace common
} // namespace subttxrend
