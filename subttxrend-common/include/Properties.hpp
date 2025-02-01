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

#include "Utils.hpp"
#include <string>
#include <map>

namespace subttxrend
{
namespace common
{

class Properties final
{
  public:

    Properties() = default;

    template <typename ...KVs>
    Properties(KVs ...props)
    {
        set(std::forward<KVs>(props)...);
    }

    std::string toString() const;

    void clear();
    bool hasKey(std::string const& k) const;
    template<class T>
    bool hasKey(T const& id) const
    {
        using common::to_string;
        using std::to_string;
        return hasKey(common::to_string(id));
    }

    std::string const& getString(std::string const& k) const;
    void setString(std::string const& key, std::string const& value);

    template <typename K, typename V, typename ...KVs>
    void set(K const& k, V const& v, KVs ...kvs)
    {
        set(k, v, std::forward<KVs>(kvs)...);
    }

    template <class T>
    void set(std::string const& k, T const& v)
    {
        using common::to_string;
        using std::to_string;

        setString(k, common::to_string(v));
    }

    template <class T>
    T get(std::string const& k) const
    {
        using common::from_string;

        return common::from_string<T>(getString(k));
    }

    template <class T>
    T get(std::string const& k, T&& defval) const
    {
        using common::from_string;

        auto it = find(k);
        if (it != end()) {
            return common::from_string<T>(getString(it));
        }
        return T{std::forward<T>(defval)};
    }

    template <class T, class U, class = typename std::enable_if<is_scoped_enum<T>::value>::type>
    void set(T const& k, U const& v)
    {
        using common::to_string;
        using std::to_string;

        setString(common::to_string(k), common::to_string(v));
    }

    template <class U, class T, class = typename std::enable_if<is_scoped_enum<T>::value>::type>
    U get(T k) const
    {
        using common::to_string;
        using common::from_string;

        return common::from_string<U>(getString(common::to_string(k)));
    }

    template <class U, class T, class = typename std::enable_if<is_scoped_enum<T>::value>::type>
    U get(T k, U&& defval) const
    {
        using common::to_string;
        using common::from_string;

        auto it = find(common::to_string(k));
        if (it != end()) {
            return common::from_string<U>(getString(it));
        }
        return U{std::forward<U>(defval)};
    }

    template<class C>
    void forEach(C&& c) const
    {
        for (auto const& kv: properties) {
            c(kv.first, kv.second);
        }
    }

  private:
    using Map = std::map<std::string, std::string>;
    using ConstIterator = Map::const_iterator;

    ConstIterator end() const;
    ConstIterator find(std::string const& k) const;
    std::string const& getString(ConstIterator it) const;

    Map properties;
};

// "k1:v1;k2:v2;k3:v3"
Properties& paramsToProperties(Properties& p, std::string const& params);

std::ostream& operator<<(std::ostream& os, Properties const& props);


} // namespace common
} // namespace subttxrend
