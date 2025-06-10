/**
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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

#ifndef SUBTTXREND_GFX_QUARTZ_BACKEND_HPP_
#define SUBTTXREND_GFX_QUARTZ_BACKEND_HPP_

#include "Backend.hpp"
#include <vector>
#include <memory>
#include <thread>
#include "Types.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Rendering backend using Quartz.
 */
class QuartzBackend : public Backend
{
    public:
        /**
         * Constructor.
         *
         * @param listener
         *      Listener for backend events.
         */
        QuartzBackend(BackendListener* listener);

        /**
         * Destructor.
         */
        virtual ~QuartzBackend();

        /** @copydoc Backend::isSyncNeeded() */
        bool isSyncNeeded() const override final;

        /** @copydoc Backend::init() */
        bool init() override final;

        /** @copydoc Backend::start() */
        bool start() override final;

        /** @copydoc Backend::stop() */
        void stop() override final;

        /** @copydoc Backend::startBlockingApplicationWindow() */
        void startBlockingApplicationWindow() override final;

        /**
         * Requests render.
         *
         * This method shall be used to request the frame to be redrawn.
         */
        void requestRender() override final;

        /**
         * Force render.
         *
         * This method shall be used to request the frame to be redrawn without
         * waiting for external conditions (like frame done callback).
         */
        void forceRender() override final;

    private:
        /** Initial window size. */
        Size m_initialSize;

        /** Buffer for clearing the image */
        std::vector<uint8_t> m_clearArrayBuffer;

        /** Default size. */
        static const Size DEFAULT_SIZE;

        /** Buffer for the data to be passed to Cocoa. */
        std::vector<uint8_t> m_buffer;

        /**
         * @brief Causes the redraw of the subtitles
         */
        void redraw();

        /**
         * @brief Calculates the content size.
         *
         * @return Calculated size.
         */
        Size calculateContentSize();

        /**
         * @brief Copies the subtitle pixmap (as bitmap) to the buffer
         *        used to send the subtitle data to the Quartz engine.
         *
         * @param contentSize   Size of the content to copy.
         *
         * @return true on success.
         * @return false on error.
         */
        bool copyToBuffer(const Size& contentSize);

        /**
         * @brief Prepares the buffer, used for copying the subtitle pixmap
         *        to the Quartz engine, by resizing it to the given content
         *        size.
         *
         * @param contentSize   The size of the content that will be copied.
         *
         * @return true on success.
         * @return false on error.
         */
        bool prepareBuffer(const Size& contentSize);

};

} // namespace gfx
} // namespace subttxrend

#endif // SUBTTXREND_GFX_QUARTZ_BACKEND_HPP_
