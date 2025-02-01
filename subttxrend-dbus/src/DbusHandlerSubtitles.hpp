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


#ifndef _SUBTTXREND_DBUS_DBUSHANDLERSUBTITLES_HPP_
#define _SUBTTXREND_DBUS_DBUSHANDLERSUBTITLES_HPP_

#include <string>

#include <glib-2.0/glib-object.h>
#include <glib-2.0/gio/gio.h>

#include <subttxrend/common/Logger.hpp>
#include <subttxrend/common/NonCopyable.hpp>

#include "DbusRequestHandler.hpp"
#include "SubtitleDbusInterface.h"

namespace subttxrend
{
namespace dbus
{

/**
 * Registers and handles request for dbus Subtitles object.
 */
class DbusHandlerSubtitles : public common::NonCopyable
{
public:

    /**
     * Constructor.
     *
     * @param requestHandler
     *      Incoming requests handler.
     */
    DbusHandlerSubtitles(DbusRequestHandler &requestHandler);

    /**
     * Destructor.
     */
    virtual ~DbusHandlerSubtitles();

    /**
     * Initialization function.
     *
     * @return
     *      True if initialization was successful, false otherwise.
     */
    bool init();

    /**
     * Cleanup function. Releases all resources held by instance. Stops exporting
     * dbus object, releases session bus.
     */
    void doCleanup();

private:

    /**
     * Registered Glib callback for setMuted dbus request.
     *
     * @param interface
     *      Subtitles GObject where request was received.
     *
     * @param invocation
     *      Dbus request call identifier.
     *
     * @param request
     *      Json encoded request data.
     *
     * @param userData
     *      Data passed during callback registration.
     *
     * @return
     *      TRUE if method completed successfully.
     */
    static gboolean onHandleSetMuted(Subtitles *interface,
                                     GDBusMethodInvocation *invocation,
                                     const gchar *request,
                                     gpointer userData);

    /**
     * Registered Glib callback for getStatus dbus request.
     *
     * @param interface
     *      Subtitles GObject where request was received.
     *
     * @param invocation
     *      Dbus request call identifier.
     *
     * @param userData
     *      Data passed during callback registration.
     *
     * @return
     *      TRUE if method completed successfully.
     */
    static gboolean onHandleGetStatus(Subtitles *interface,
                                      GDBusMethodInvocation *invocation,
                                      gpointer userData);

    /**
     * Invoked when a connection to a message bus has been obtained.
     *
     * @param connection
     *      The GDBusConnection to a message bus.
     *
     * @param name
     *      The name that is requested to be owned.
     *
     * @param userData
     *      User data passed to g_bus_own_name().
     */
    static void onBusAcquired(GDBusConnection *connection,
                              const gchar *name,
                              gpointer userData);

    /**
     * Invoked when the name is acquired.
     *
     * @param connection
     *      The GDBusConnection on which to acquired the name.
     *
     * @param name
     *      The name being owned.
     *
     * @param userData
     *      User data passed to g_bus_own_name().
     */
    static void onNameAcquired(GDBusConnection *connection,
                               const gchar *name,
                               gpointer userData);

    /**
     * Invoked when the name is lost or connection has been closed.
     *
     * @param connection
     *      The GDBusConnection on which to acquired the name.
     *
     * @param name
     *      The name being owned.
     *
     * @param userData
     *      User data passed to g_bus_own_name().
     */
    static void onNameLost(GDBusConnection *connection,
                           const gchar *name,
                           gpointer userData);

    /**
     * Setter for setMuted signal handler.
     */
    void setSetMutedSignalHandle(gulong setMutedSignalHandle)
    {
        m_setMutedSignalHandle = setMutedSignalHandle;
    }

    /**
     * Setter for getStatus signal handler.
     */
    void setGetStatusSignalHandle(gulong getStatusSignalHandle)
    {
        m_getStatusSignalHandle = getStatusSignalHandle;
    }

    /**
     * Setter for Subtitle gObject.
     */
    void setSubtitlesGObj(Subtitles* subtitlesGObj)
    {
        m_subtitlesGObj = subtitlesGObj;
    }

    /**
     * Process set muted request.
     *
     * @param request
     *      String containing request data.
     */
    void handleSetMuted(const std::string &request);

    /**
     * Process getStatus request.
     *
     * @return
     *      String containing status data.
     */
    std::string handleGetStatus();

    /** logger instance */
    static common::Logger m_logger;

    /** Registered request handler. */
    DbusRequestHandler& m_requestHandler;

    /** dbus owner id. */
    guint m_busOwnerId;

    /** Registered Subtitles dbus object (nullptr if not yet registered). */
    Subtitles* m_subtitlesGObj;

    /** setMuted dbus signal handler. */
    gulong m_setMutedSignalHandle;

    /** getStatus dbus signal handler. */
    gulong m_getStatusSignalHandle;

};

} // namespace dbus
} // namespace subttxrend

#endif /* _SUBTTXREND_DBUS_DBUSHANDLERSUBTITLES_HPP_ */
