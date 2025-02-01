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


#ifndef SUBTTXREND_GFX_BACKEND_HPP_
#define SUBTTXREND_GFX_BACKEND_HPP_

#include "BackendListener.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Rendering backend interface.
 */
class Backend
{
public:
    /**
     * Constructor.
     */
    Backend(BackendListener* listener) :
            m_listener(listener)
    {
        // noop
    }

    /**
     * Destructor.
     */
    virtual ~Backend() = default;

    /**
     * Checks if synchronization is needed.
     *
     * @return
     *      True if synchronization is needed, false otherwise.
     */
    virtual bool isSyncNeeded() const = 0;

    /**
     * Initializes the backend.
     *
     * @retval true
     *      Success.
     * @retval false
     *      Error.
     */
    virtual bool init(const std::string &displayName) = 0;

    /**
     * Starts the backend.
     *
     * @retval true
     *      Success.
     * @retval false
     *      Error.
     */
    virtual bool start() = 0;

    /**
     * Stops the backend.
     */
    virtual void stop() = 0;

    /**
     * Requests render.
     *
     * This method shall be used to request the frame to be redrawn.
     */
    virtual void requestRender() = 0;

    /**
     * Forces render.
     *
     * This method shall be used to force the frame to be redrawn without waiting for external
     * conditions (like frame done callback).
     */
    virtual void forceRender() = 0;

protected:
    /**
     * Returns backend events listener.
     *
     * @return
     *      Listener object.
     */
    BackendListener* getListener() const
    {
        return m_listener;
    }

private:
    /** Backend events listener. */
    BackendListener* const m_listener;
};

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_BACKEND_HPP_
