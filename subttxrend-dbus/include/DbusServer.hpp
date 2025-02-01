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


#ifndef _SUBTTXREND_DBUS_DBUSSERVER_HPP_
#define _SUBTTXREND_DBUS_DBUSSERVER_HPP_

#include <memory>           // for unique_ptr

#include <subttxrend/common/NonCopyable.hpp>

namespace subttxrend
{
namespace dbus
{

class Executor;
class DbusServerImpl;

/**
 * Server listening on dbus teletext and subtitle control messages.
 *
 * Incoming request are parsed and passed to registered executor.
 */
class DbusServer : private common::NonCopyable
{
public:

    /**
     * Constructor
     *
     * @param executor
     *      Dbus requests executor.
     */
    DbusServer(Executor &executor);

    /**
     * Initialization function. Starts dbus registration process. Registration finishes
     * in callbacks from glib main loop.
     *
     * @return
     *      True if initialization was successful, false otherwise.
     */
    bool init();

    /**
     * Destructor.
     */
    virtual ~DbusServer();

private:
    /** Pointer to implementation. */
    std::unique_ptr<DbusServerImpl> m_impl;
};

} // namespace dbus
} // namespace subttxrend

#endif /* _SUBTTXREND_DBUS_DBUSSERVER_HPP_ */
