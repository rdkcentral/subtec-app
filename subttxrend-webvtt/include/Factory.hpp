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

#include "WebvttEngine.hpp"

namespace subttxrend
{
namespace webvttengine
{

/**
 * Factory for creating webvtt engine.
 */
class Factory
{
    Factory() = delete;

public:
    /**
     * Creates webvtt subtitles engine.
     *
     * @return
     *      Created engine.
     */
    static std::unique_ptr<WebvttEngine> createWebvttEngine();

};

} // namespace webvttengine
} // namespace subttxrend
