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


#ifndef SUBTTXREND_COMMON_NONCOPYABLE_HPP_
#define SUBTTXREND_COMMON_NONCOPYABLE_HPP_

#include <string>

namespace subttxrend
{
namespace common
{

/**
 * Non-copyable marker class.
 *
 * It could be used (by inheritance) to block copying of objects of
 * specific class.
 */
class NonCopyable
{
public:
    NonCopyable() = default;
    virtual ~NonCopyable() = default;

    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_NONCOPYABLE_HPP_*/
