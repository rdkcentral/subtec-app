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


#include "Application.hpp"

#include <cstring>
#include <string>
#include <unistd.h>

#include <subttxrend/common/Logger.hpp>
#include <subttxrend/common/LoggerManager.hpp>
#include <subttxrend/socksrc/UnixSocketSourceFactory.hpp>
#include <subttxrend/protocol/Packet.hpp>
#include <subttxrend/dbus/DbusServer.hpp>
#include <subttxrend/gfx/Factory.hpp>

namespace subttxrend
{
namespace app
{

Application::Application(const Options& options) :
        m_configuration(options),
        m_logger("App", "Application", this)
{
    // noop
}

void Application::runAsync()
{
    m_logger.osinfo(__LOGGER_FUNC__, " - started.");

    common::LoggerManager::getInstance()->init(&m_configuration.getLoggerConfig());

    m_logger.osinfo(__LOGGER_FUNC__, " - Logger initialized.");
    m_logger.osinfo(__LOGGER_FUNC__, " - Starting application.");

    m_logger.osinfo(__LOGGER_FUNC__, " - Initialize graphics engine.");

    m_gfxEngine = gfx::Factory::createEngine();
    m_gfxEngine->init();

    m_gfxWindow = m_gfxEngine->createWindow();
    m_gfxEngine->attach(m_gfxWindow);

    m_logger.osinfo(__LOGGER_FUNC__, " - window created ", m_gfxWindow.get(), ", initializing controller");

    m_region = m_configuration.getRegionInfo();

    m_controller = std::make_unique<Controller>(m_configuration, m_gfxEngine, m_gfxWindow, m_region);

    auto socketPath = m_configuration.getMainContextSocketPath();
    m_logger.osinfo( __LOGGER_FUNC__, " - Creating unix socket source with path='", socketPath, "'.");

    m_source = socksrc::UnixSocketSourceFactory().create(socketPath);
    if (!m_source) {
        m_logger.osfatal(__LOGGER_FUNC__, " - Cannot create socket source");
        throw std::runtime_error("error while creating source");
    }

    m_logger.osinfo(__LOGGER_FUNC__, " - Starting source");
    m_source->start(m_controller.get());

    m_logger.osinfo(__LOGGER_FUNC__, " - Starting render thread");
    m_controller->startAsync();

    m_logger.osinfo( __LOGGER_FUNC__, " - application started");
}

void Application::quit()
{
    m_logger.osinfo(__LOGGER_FUNC__);

    m_source->stop();
    m_controller->stop();

    m_logger.osinfo(__LOGGER_FUNC__, "shutting down gfx... ");
    m_gfxEngine->detach(m_gfxWindow);
    m_gfxWindow.reset();
    m_gfxEngine->shutdown();

    m_logger.osinfo(__LOGGER_FUNC__, "application stopped, stopping logger... ");

    common::LoggerManager::getInstance()->deinit();
}

} // namespace app
} // namespace subttxrend
