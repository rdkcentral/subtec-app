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


#ifndef _SUBTTXREND_DBUS_EXECUTOR_HPP_
#define _SUBTTXREND_DBUS_EXECUTOR_HPP_

#include "SubtitleStatus.hpp"
#include "TeletextStatus.hpp"

namespace subttxrend
{
namespace dbus
{

/**
 * Interface for listening for subtitle and teletext requests.
 *
 */
class Executor
{
public:

    /**
     * Destructor.
     */
    virtual ~Executor() = default;

    /**
     * Listener for subtitle "setMuted" requests.
     *
     * @param muted
     */
    virtual void subtitleSetMuted(bool muted) = 0;

    /**
     * Listener for subtitle "getStatus" requests.
     *
     * @return
     *      Subtitle status.
     */
    virtual SubtitleStatus subtitleGetStatus() = 0;

    /**
     * Listener for teletext "setMuted" requests.
     *
     * @param muted
     */
    virtual void teletextSetMuted(bool muted) = 0;

    /**
     * Listener for teletext "getStatus" requests.
     *
     * @return
     *      Teletext status data.
     */
    virtual TeletextStatus teletextGetStatus() = 0;
};

} // namespace dbus
} // namespace subttxrend

#endif /* _SUBTTXREND_DBUS_EXECUTOR_HPP_ */
