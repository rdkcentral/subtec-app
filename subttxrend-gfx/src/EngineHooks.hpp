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


#ifndef SUBTTXREND_GFX_ENGINE_HOOKS_HPP_
#define SUBTTXREND_GFX_ENGINE_HOOKS_HPP_

namespace subttxrend
{
namespace gfx
{

/**
 * Window engine hooks.
 */
class EngineHooks
{
public:
    /**
     * Constructor.
     */
    EngineHooks() = default;

    /**
     * Destructor.
     */
    virtual ~EngineHooks() = default;

    /**
     * Requests redraw.
     */
    virtual void requestRedraw() = 0;

    /**
     * Forces redraw.
     */
    virtual void forceRedraw() = 0;

    /**
     * Locks (synchronization) the engine.
     */
    virtual void lock() = 0;

    /**
     * Unlock (synchronization) the engine.
     */
    virtual void unlock() = 0;

};

} // namespace gfx
} // namespace subttxrend

#endif /*SUBTTXREND_GFX_ENGINE_HOOKS_HPP_*/
