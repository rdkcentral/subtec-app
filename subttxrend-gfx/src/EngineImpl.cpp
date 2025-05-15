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


#include "EngineImpl.hpp"

#include <algorithm>

#include <subttxrend/common/Logger.hpp>

#include "BackendFactory.hpp"
#include "FontStripImpl.hpp"

namespace subttxrend
{
namespace gfx
{

namespace
{

common::Logger g_logger("Gfx", "EngineImpl");

}

EngineImpl::EngineImpl()
{
    g_logger.trace("%s", __func__);
}

EngineImpl::~EngineImpl()
{
    g_logger.trace("%s", __func__);

    if (m_attachedWindows.size() > 0)
    {
        g_logger.fatal("%s - Not all windows detached", __func__);
    }
}

void EngineImpl::init(const std::string &displayName)
{
    g_logger.trace("%s", __func__);

    auto backend = BackendFactory::createBackend(this);
    if (backend->init(displayName))
    {
        if (backend->start())
        {
            m_backend = std::move(backend);

            g_logger.info("%s - Backend is running", __func__);
        }
        else
        {
            g_logger.fatal("%s - Cannot run backend", __func__);
        }
    }
    else
    {
        g_logger.fatal("%s - Cannot init backend", __func__);
    }
}

void EngineImpl::shutdown()
{
    if (m_backend)
    {
        m_backend->stop();
        m_backend.reset();
    }
}

void EngineImpl::execute()
{
    lock();

    while (!m_keyEventsQueue.empty())
    {
        // pass to first visible window
        for (auto& window : m_attachedWindows)
        {
            if (window->isVisible())
            {
                window->processKeyEvent(m_keyEventsQueue.front());
                break;
            }
        }

        m_keyEventsQueue.pop();
    }

    unlock();
}

WindowPtr EngineImpl::createWindow()
{
    g_logger.trace("%s", __func__);

    auto windowImpl = std::make_shared<WindowImpl>();

    g_logger.trace("%s -> %p", __func__, windowImpl.get());

    return windowImpl;
}

FontStripPtr EngineImpl::createFontStrip(const Size& glyphSize,
                                         const std::size_t glyphCount)
{
    g_logger.trace("%s size=%dx%d count=%zu", __func__, glyphSize.m_w,
            glyphSize.m_h, glyphCount);

    return std::make_shared<FontStripImpl>(glyphSize, glyphCount);
}

void EngineImpl::attach(WindowPtr window)
{
    if (!window)
    {
        throw std::invalid_argument("window");
    }

    lock();

    auto windowImpl = std::static_pointer_cast<WindowImpl>(window);

    g_logger.trace("%s - window=%p", __func__, windowImpl.get());

    auto iter = std::find(m_attachedWindows.begin(), m_attachedWindows.end(),
            windowImpl);
    if (iter != m_attachedWindows.end())
    {
        unlock();
        throw std::logic_error("Window already attached");
    }

    m_attachedWindows.push_back(windowImpl);

    windowImpl->setEngineHooks(this);

    unlock();
}

void EngineImpl::detach(WindowPtr window)
{
    if (!window)
    {
        throw std::invalid_argument("window");
    }

    lock();

    auto windowImpl = std::static_pointer_cast<WindowImpl>(window);

    g_logger.trace("%s - window=%p", __func__, windowImpl.get());

    auto iter = std::find(m_attachedWindows.begin(), m_attachedWindows.end(),
            windowImpl);
    if (iter != m_attachedWindows.end())
    {
        windowImpl->setEngineHooks(nullptr);
        m_attachedWindows.erase(iter);
    }
    else
    {
        g_logger.warning("%s - window not attached", __func__);
    }

    unlock();
}

void EngineImpl::requestRedraw()
{
    if (m_backend)
    {
        m_backend->requestRender();
    }
}

void EngineImpl::forceRedraw()
{
    if (m_backend)
    {
        m_backend->forceRender();
    }
}

void EngineImpl::lock()
{
    if (m_backend && !m_backend->isSyncNeeded())
    {
        return;
    }

    m_mutex.lock();
}

void EngineImpl::unlock()
{
    if (m_backend && !m_backend->isSyncNeeded())
    {
        return;
    }

    m_mutex.unlock();
}

void EngineImpl::enumerateVisibleWindows(BackendWindowEnumerator& enumerator)
{
    g_logger.trace("%s", __func__);

    lock();

    for (auto& window : m_attachedWindows)
    {
        if (window->isVisible())
        {
            const auto& pixmap = window->getPixmap();

            g_logger.trace("%s -> wnd=%p pix=%dx%d", __func__, window.get(),
                pixmap.getWidth(), pixmap.getHeight());
                
#if BACKEND_TYPE == BACKEND_TYPE_EGL
            auto& bgpixmap = window->getBgPixmap();
            enumerator.processWindow(pixmap, bgpixmap);
#else
            enumerator.processWindow(pixmap);
#endif
        }
    }

    unlock();
}

void EngineImpl::focusGained()
{
    // noop
}

void EngineImpl::focusLost()
{
    // noop
}

void EngineImpl::processKeyEvent(const KeyEvent& event)
{
    g_logger.trace("%s - type=%d symbol=%u", __func__,
            static_cast<int>(event.getType()), event.getSymbol());

    lock();
    m_keyEventsQueue.push(event);
    unlock();
}

void EngineImpl::onPreferredSize(const Size& size)
{
    g_logger.info("%s - w=%d h=%d", __func__, size.m_w, size.m_h);

    lock();

    for (auto& window : m_attachedWindows)
    {
        window->setPreferredSize(size);
    }

    unlock();
}

#ifdef __APPLE__
void EngineImpl::startBlockingApplicationWindow()
{
    if (m_backend)
    {
        m_backend->startBlockingApplicationWindow();
    }
}
#endif

}
 // namespace gfx
}// namespace subttxrend
