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


#ifndef _SUBTTXREND_DBUS_SRC_DBUSHANDLER_HPP_
#define _SUBTTXREND_DBUS_SRC_DBUSHANDLER_HPP_

#include <string>

#include <glib-2.0/glib-object.h>
#include <glib-2.0/gio/gio.h>

#include <subttxrend/common/Logger.hpp>
#include <subttxrend/common/NonCopyable.hpp>

#include "DbusHandlerSubtitles.hpp"
#include "DbusHandlerTeletext.hpp"

namespace subttxrend
{
namespace dbus
{

/**
 * Class responsible for dbus communication.
 */
class DbusRequestHandler;

/**
 * DBus communication handler.
 */
class DbusHandler : public common::NonCopyable
{
public:

    /**
     * Constructor.
     *
     * @param requestHandler
     *      Incoming requests handler.
     */
    DbusHandler(DbusRequestHandler &requestHandler);

    /**
     * Destructor.
     */
    virtual ~DbusHandler();

    /**
     * Initialization function.
     *
     * @return
     *      True if initialization was successful, false otherwise.
     */
    bool init();

private:

    /** logger instance */
    common::Logger m_logger;

    /** Subtitles dbus object handler. */
    DbusHandlerSubtitles m_subtitlesHandler;

    /** Teletext dbus object handler. */
    DbusHandlerTeletext m_teletextHandler;
};

} // namespace dbus
} // namespace subttxrend

#endif /* _SUBTTXREND_DBUS_SRC_DBUSHANDLER_HPP_ */
