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


#ifndef SUBTTXREND_GFX_BACKEND_LISTENER_HPP_
#define SUBTTXREND_GFX_BACKEND_LISTENER_HPP_

#include <cstdint>

#include "KeyEvent.hpp"
#include "Types.hpp"

namespace subttxrend
{
namespace gfx
{

class Pixmap;
/**
 * Window enumeration interface.
 */
class BackendWindowEnumerator
{
public:
    /**
     * Destructor
     */
    virtual ~BackendWindowEnumerator() = default;

    /**
     * Process enumerated window.
     *
     * @param pixmap
     *      Window pixmap.
     */
    virtual void processWindow(const Pixmap& pixmap) = 0;
    virtual void processWindow(const Pixmap& pixmap, const Pixmap& bgPixmap){std::exit(0);};
};

/**
 * Listener for backend events.
 */
class BackendListener
{
public:
    /**
     * Constructor.
     */
    BackendListener() = default;

    /**
     * Destructor.
     */
    virtual ~BackendListener() = default;

    /**
     * Requests enumeration of visible windows.
     *
     * @note The enumeration may be called under synchronization
     *       so make sure there will be no deadlock when calling this method.
     *
     * @param enumerator
     *      Interface to be called during enumerations.
     */
    virtual void enumerateVisibleWindows(BackendWindowEnumerator& enumerator) = 0;

    /**
     * Notifies that focus was gained.
     */
    virtual void focusGained() = 0;

    /**
     * Notifies that focus was lost.
     */
    virtual void focusLost() = 0;

    /**
     * Processes key event.
     *
     * @param event
     *      Event descriptor.
     */
    virtual void processKeyEvent(const KeyEvent& event) = 0;

    /**
     * Notifies preferred window size.
     *
     * @param size
     *      Preferred window size.
     */
    virtual void onPreferredSize(const Size& size) = 0;

};

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_BACKEND_LISTENER_HPP_
