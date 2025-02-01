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


#ifndef SUBTTXREND_GFX_WAYLAND_LOOP_GLIB_HPP_
#define SUBTTXREND_GFX_WAYLAND_LOOP_GLIB_HPP_

#include <glib.h>

#include "WaylandBackendLoop.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Backend loop - Glib source based.
 */
class WaylandBackendLoopGlib : public WaylandBackendLoop
{
public:
    /**
     * Constructor.
     */
    WaylandBackendLoopGlib();

    /**
     * Destructor.
     */
    virtual ~WaylandBackendLoopGlib();

    virtual bool isSyncNeeded() const override
    {
        return false;
    }

    virtual bool start(waylandcpp::DisplayPtr display,
                       WaylandBackendLoopListener* listener) override;

    virtual void stop() override;

    virtual void requestWakeup() override;

protected:
    /** GLib source. */
    GSource* m_source;
};

}
// namespace gfx
}// namespace subttxrend

#endif                          // SUBTTXREND_GFX_WAYLAND_LOOP_GLIB_HPP_
