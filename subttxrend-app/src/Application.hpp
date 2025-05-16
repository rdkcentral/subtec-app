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


#ifndef SUBTTXREND_APP_APPLICATION_HPP_
#define SUBTTXREND_APP_APPLICATION_HPP_

#include <memory>

#include <glib.h>

#include <subttxrend/common/NonCopyable.hpp>
#include <subttxrend/common/Logger.hpp>
#include <subttxrend/socksrc/Source.hpp>
#include <subttxrend/gfx/Engine.hpp>

#include "Controller.hpp"

namespace subttxrend
{
namespace app
{

/**
 * Main application class.
 */
class Application : private common::NonCopyable
{
public:

    Application(const ctrl::Options& options);
    virtual ~Application() = default;

    void runAsync();
    void quit();

#ifdef __APPLE__
    void startBlockingApplicationWindow();
#endif

private:

    /** Application configuration. */
    ctrl::Configuration m_configuration;

    /** Unix socket path to use. */
    const std::string m_socketPath;

    /** Data source. */
    socksrc::SourcePtr m_source;

    /** Graphics engine. */
    gfx::EnginePtr m_gfxEngine;

    /** Application window. */
    gfx::WindowPtr m_gfxWindow;

    /** Subtitle and teletext control object. */
    ControllerPtr m_controller;
    common::Logger m_logger;
};

} // namespace app
} // namespace subttxrend

#endif /*SUBTTXREND_APP_APPLICATION_HPP_*/
