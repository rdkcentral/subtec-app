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


#ifndef TTXDECODER_ENGINEFACTORY_HPP_
#define TTXDECODER_ENGINEFACTORY_HPP_

#include <memory>

#include "Allocator.hpp"
#include "Engine.hpp"
#include "EngineClient.hpp"

namespace ttxdecoder
{

/**
 * Engine factory.
 */
class EngineFactory
{
    EngineFactory() = delete;

public:
    /**
     * Creates the engine.
     *
     * @param client
     *      Client interface.
     * @param allocator
     *      Allocator to use.
     *
     * @return
     *      Created engine or null on error.
     */
    static std::unique_ptr<Engine> createEngine(EngineClient& client,
                                                std::unique_ptr<Allocator> allocator);
};

} // namespace ttxdecoder

#endif /*TTXDECODER_ENGINEFACTORY_HPP_*/
