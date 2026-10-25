/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
 */

#pragma once

#include <memory>
#include <string>

#include <subttxrend/gfx/FontStrip.hpp>
#include <subttxrend/gfx/Window.hpp>

namespace subttxrend
{
namespace gfx
{

class Engine
{
public:
    Engine() = default;
    virtual ~Engine() = default;

    virtual void init(const std::string& displayName = {}) = 0;
    virtual void shutdown() = 0;
    virtual void execute() = 0;
    virtual WindowPtr createWindow() = 0;
    virtual FontStripPtr createFontStrip(const Size& glyphSize,
                                         const std::size_t glyphCount) = 0;
    virtual void attach(WindowPtr window) = 0;
    virtual void detach(WindowPtr window) = 0;

#ifdef __APPLE__
    virtual void startBlockingApplicationWindow() = 0;
#endif
};

using EnginePtr = std::shared_ptr<Engine>;

} // namespace gfx
} // namespace subttxrend