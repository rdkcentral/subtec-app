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


#include <memory>

#include "DbusHandlerSubtitles.hpp"

#include "DbusRequestHandler.hpp"

namespace subttxrend
{
namespace dbus
{

namespace // anonymous
{
const char* DBUS_NAME = "com.libertyglobal.rdk.subtitles";
const char* DBUS_PATH = "/com/libertyglobal/rdk/subtitles";

// invalid signal handle
constexpr gulong INVALID_SIGNAL_HANDLE = 0;

// invalid bus owner id
constexpr guint DBUS_OWNER_ID_NONE = 0;

} // namespace anonymous

common::Logger DbusHandlerSubtitles::m_logger("Dbus", "DbusHandlerSubtitles");

gboolean DbusHandlerSubtitles::onHandleSetMuted(Subtitles *interface,
                                                GDBusMethodInvocation *invocation,
                                                const gchar *request,
                                                gpointer userData)
{
    m_logger.trace("%s: interface=%p invocation=%p request=%s data=%p", __func__, interface, invocation, request,
            userData);

    DbusHandlerSubtitles* thiz = static_cast<DbusHandlerSubtitles*>(userData);
    thiz->handleSetMuted(request);

    subtitles_complete_set_muted(interface, invocation);
    return TRUE;
}

gboolean DbusHandlerSubtitles::onHandleGetStatus(Subtitles *interface,
                                                 GDBusMethodInvocation *invocation,
                                                 gpointer userData)
{
    m_logger.trace("%s: interface=%p invocation=%p data=%p", __func__, interface, invocation, userData);

    DbusHandlerSubtitles* thiz = static_cast<DbusHandlerSubtitles*>(userData);
    std::string response  = thiz->handleGetStatus();

    subtitles_complete_get_status(interface, invocation, response.c_str());
    return TRUE;
}

void DbusHandlerSubtitles::onBusAcquired(GDBusConnection *connection,
                                         const gchar *name,
                                         gpointer userData)
{
    m_logger.trace("%s: connection=%p name=%s data=%p", __func__, connection, name, userData);

    DbusHandlerSubtitles* thiz = static_cast<DbusHandlerSubtitles*>(userData);

    Subtitles* subtitlesObj = subtitles_skeleton_new();

    if (subtitlesObj == nullptr)
    {
        m_logger.error("Could not create gObject: subtitles_skeleton_new failed");
        return;
    }

    thiz->setSubtitlesGObj(subtitlesObj);

    gulong setMutedSignalHandler = g_signal_connect(subtitlesObj, "handle_set_muted", G_CALLBACK(onHandleSetMuted),
            userData);
    if (setMutedSignalHandler == INVALID_SIGNAL_HANDLE)
    {
        m_logger.error("Could not register Subtitles handle_set_muted signal");
        thiz->doCleanup();
        return;
    }

    thiz->setSetMutedSignalHandle(setMutedSignalHandler);

    gulong getStatusSignalHandler = g_signal_connect(subtitlesObj, "handle_get_status", G_CALLBACK(onHandleGetStatus),
            userData);
    if (getStatusSignalHandler == INVALID_SIGNAL_HANDLE)
    {
        m_logger.error("Could not register Subtitles handle_get_status signal");
        thiz->doCleanup();
        return;
    }

    thiz->setGetStatusSignalHandle(getStatusSignalHandler);

    GError *gError = nullptr;
    if (!g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(subtitlesObj), connection, DBUS_PATH, &gError))
    {
        m_logger.error("%s: could not export gObject", __func__);
        thiz->doCleanup();
        return;
    }

    m_logger.info("%s: gObject exported", __func__);
}

void DbusHandlerSubtitles::onNameAcquired(GDBusConnection *connection,
                                          const gchar *name,
                                          gpointer userData)
{
    m_logger.trace("Acquired name %s on the system bus, connection=%p", name, connection);
}

void DbusHandlerSubtitles::onNameLost(GDBusConnection *connection,
                                      const gchar *name,
                                      gpointer userData)
{
    m_logger.error("Lost the name %s on the system bus, connection=%p", name, connection);
}

DbusHandlerSubtitles::DbusHandlerSubtitles(DbusRequestHandler &requestHandler) :
        m_requestHandler(requestHandler),
        m_busOwnerId(DBUS_OWNER_ID_NONE),
        m_subtitlesGObj(nullptr),
        m_setMutedSignalHandle(INVALID_SIGNAL_HANDLE),
        m_getStatusSignalHandle(INVALID_SIGNAL_HANDLE)
{
    m_logger.info("created");
}
;

bool DbusHandlerSubtitles::init()
{
    m_busOwnerId = g_bus_own_name(G_BUS_TYPE_SYSTEM, DBUS_NAME, G_BUS_NAME_OWNER_FLAGS_NONE, onBusAcquired,
            onNameAcquired, onNameLost, this, nullptr);

    if (m_busOwnerId == DBUS_OWNER_ID_NONE)
    {
        m_logger.error("%s: Error cannot get bus", __func__);
        return false;
    }

    m_logger.info("init: bus ownerId=%d", m_busOwnerId);

    return true;
}

void DbusHandlerSubtitles::doCleanup()
{
    if (m_subtitlesGObj != nullptr)
    {
        if (m_getStatusSignalHandle != INVALID_SIGNAL_HANDLE)
        {
            m_logger.trace("releasing getStatus handle %lu...", m_getStatusSignalHandle);
            g_signal_handler_disconnect(m_subtitlesGObj, m_getStatusSignalHandle);
            m_getStatusSignalHandle = INVALID_SIGNAL_HANDLE;
        }

        if (m_setMutedSignalHandle != INVALID_SIGNAL_HANDLE)
        {
            m_logger.trace("releasing setMuted handle %lu...", m_setMutedSignalHandle);
            g_signal_handler_disconnect(m_subtitlesGObj, m_setMutedSignalHandle);
            m_setMutedSignalHandle = INVALID_SIGNAL_HANDLE;
        }

        m_logger.trace("unexporting skeleton ...");

        g_dbus_interface_skeleton_unexport(G_DBUS_INTERFACE_SKELETON(m_subtitlesGObj));

        m_logger.trace("releasing m_subtitlesGObj %p...", m_subtitlesGObj);

        g_object_unref(m_subtitlesGObj);
        m_subtitlesGObj = nullptr;
    }

    if (m_busOwnerId != DBUS_OWNER_ID_NONE)
    {
        m_logger.trace("unowning %d...", m_busOwnerId);
        g_bus_unown_name(m_busOwnerId);
        m_busOwnerId = DBUS_OWNER_ID_NONE;
    }

    m_logger.info("cleanup done");
}

void DbusHandlerSubtitles::handleSetMuted(const std::string &request)
{
    m_requestHandler.subtitleSetMuted(request);
}

std::string DbusHandlerSubtitles::handleGetStatus()
{
    return m_requestHandler.subtitleGetStatus();
}

DbusHandlerSubtitles::~DbusHandlerSubtitles()
{
    doCleanup();
}

} // namespace dbus
} // namespace subttxrend

