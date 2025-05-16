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


#ifndef SUBTTXREND_GFX_ENGINE_IMPL_HPP_
#define SUBTTXREND_GFX_ENGINE_IMPL_HPP_

#include <vector>
#include <queue>
#include <mutex>

#include "Engine.hpp"
#include "WindowImpl.hpp"
#include "BackendListener.hpp"
#include "EngineHooks.hpp"

namespace subttxrend
{
namespace gfx
{

class Backend;

/**
 * Graphics engine - implementation
 */
class EngineImpl : public Engine,
                   private BackendListener,
                   private EngineHooks
{
public:
    /**
     * Constructor.
     */
    EngineImpl();

    /**
     * Destructor.
     */
    virtual ~EngineImpl();

    virtual void init(const std::string &displayName) override;

    virtual void shutdown() override;

    virtual void execute();

    virtual WindowPtr createWindow() override;

    virtual FontStripPtr createFontStrip(const gfx::Size& glyphSize,
                                         const std::size_t glyphCount) override;

    virtual void attach(WindowPtr window) override;

    virtual void detach(WindowPtr window) override;

#ifdef __APPLE__
    virtual void startBlockingApplicationWindow() override;
#endif

private:
    /** Mutex type. */
    using Mutex = std::recursive_mutex;

    virtual void requestRedraw() override;

    virtual void forceRedraw() override;

    virtual void lock() override;

    virtual void unlock() override;

    virtual void enumerateVisibleWindows(BackendWindowEnumerator& enumerator)
            override;

    virtual void focusGained() override;

    virtual void focusLost() override;

    virtual void processKeyEvent(const KeyEvent& event) override;

    virtual void onPreferredSize(const Size& size) override;

    /** Mutex for synchronization. */
    Mutex m_mutex;

    /** Collection of attached windows. */
    std::vector<WindowImplPtr> m_attachedWindows;

    /** Rendering backend. */
    std::unique_ptr<Backend> m_backend;

    /** Queue with events. */
    std::queue<KeyEvent> m_keyEventsQueue;
};

} // namespace gfx
} // namespace subttxrend

#endif /*SUBTTXREND_GFX_ENGINE_IMPL_HPP_*/
