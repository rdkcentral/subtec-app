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


#ifndef _SUBTTXREND_DBUS_DBUSSERVERIMPL_HPP_
#define _SUBTTXREND_DBUS_DBUSSERVERIMPL_HPP_

#include <memory>

#include <subttxrend/common/Logger.hpp>

#include "DbusHandler.hpp"
#include "DbusRequestHandler.hpp"
#include "Executor.hpp"

namespace subttxrend
{
namespace dbus
{

/**
 * DbusServer implementation responsible for interaction with dbus.
 *
 * Handles dbus communication, processes data and calls registered executor.
 *
 */
class DbusServerImpl : public DbusRequestHandler
{
public:

    /**
     * Constructor.
     *
     * @param executor
     *      Executor interface for handling dbus requests.
     */
    DbusServerImpl(Executor &executor);

    /**
     * Destructor.
     */
    virtual ~DbusServerImpl() = default;

    /**
     * Initialization function.
     *
     * @return
     *      True if initialization was successful, false otherwise.
     */
    bool init();

    /** @copydoc DbusRequestHandler::subtitleSetMuted() */
    virtual void subtitleSetMuted(const std::string &request) override;

    /** @copydoc DbusRequestHandler::subtitleGetStatus() */
    virtual std::string subtitleGetStatus() override;

    /** @copydoc DbusRequestHandler::teletextSetMuted() */
    virtual void teletextSetMuted(const std::string &request) override;

    /** @copydoc DbusRequestHandler::teletextGetStatus() */
    virtual std::string teletextGetStatus() override;

private:

    /** Executor interface for handling requests. */
    Executor &m_executor;

    /** Logger object. */
    common::Logger m_logger;

    /** Entity responsible for direct interaction with dbus. */
    DbusHandler m_dbusHandler;
};

} // namespace dbus
} // namespace subttxrend

#endif //_SUBTTXREND_DBUS_DBUSSERVERIMPL_HPP_
