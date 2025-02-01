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


#ifndef _SUBTTXREND_DBUS_DBUSREQUESTHANDLER_HPP_
#define _SUBTTXREND_DBUS_DBUSREQUESTHANDLER_HPP_

#include <string>

namespace subttxrend
{
namespace dbus
{

/**
 * Interface for handling incoming raw dbus requests for subtitles and teletext.
 *
 */
class DbusRequestHandler
{
public:

    /**
     * Destructor.
     */
    virtual ~DbusRequestHandler() = default;

    /**
     * Listener for subtitle "setMuted" method dbus requests.
     *
     * @param request
     *      Request data encoded in json formatted string..
     */
    virtual void subtitleSetMuted(const std::string &request) = 0;

    /**
     * Listener for subtitle "getStatus" method dbus requests.
     *
     * @return
     *      Subtitle status encoded in json formatted string.
     */
    virtual std::string subtitleGetStatus() = 0;

    /**
     * Listener for teletext "setMuted" requests.
     *
     * @param request
     *      Request data encoded in json formatted string.
     */
    virtual void teletextSetMuted(const std::string &request) = 0;

    /**
     * Listener for teletext "getStatus" method dbus requests.
     *
     * @return
     *      Teletext status encoded in json formatted string.
     */
    virtual std::string teletextGetStatus() = 0;
};

} // namespace dbus
} // namespace subttxrend

#endif /* _SUBTTXREND_DBUS_DBUSREQUESTHANDLER_HPP_ */
