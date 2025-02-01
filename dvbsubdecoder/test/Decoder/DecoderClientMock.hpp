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


#ifndef DVBSUBDECODER_TEST_DECODERCLIENT_MOCK_HPP
#define DVBSUBDECODER_TEST_DECODERCLIENT_MOCK_HPP

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>

#include "dvbsubdecoder/DecoderClient.hpp"
#include "dvbsubdecoder/TimeProvider.hpp"
#include "dvbsubdecoder/Types.hpp"
#include "dvbsubdecoder/DynamicAllocator.hpp"

using namespace dvbsubdecoder;

struct MethodData
{
    enum class Method
    {
        getStc, gfxSetDisplayBounds, gfxDraw, gfxClear, gfxFinish, gfxAllocate, gfxFree
    };

    Method method;
    union
    {
        struct
        {
            Rectangle displayBounds;
            Rectangle windowBounds;
        } gfxSetDisplayBoundsArgs;
        struct
        {
            Bitmap bitmap;
            Rectangle srcRect;
            Rectangle dstRect;
        } gfxDrawArgs;
        struct
        {
            Rectangle rect;
        } gfxClearArgs;
        struct
        {
            Rectangle rect;
        } gfxFinishArgs;
    };

    bool operator<(const MethodData& rhs) const
    {
        return method < rhs.method;
    }
};

inline std::ostream& operator <<(std::ostream& stream,
                                 const Rectangle& rect)
{
    stream << "{" << rect.m_x1 << "," << rect.m_y1 << "," << rect.m_x2 << "," << rect.m_y2 << "}";
    return stream;
}

inline std::ostream& operator <<(std::ostream& stream,
                                 const Bitmap& bitmap)
{
    stream << "{ w:" << bitmap.m_width << ", h: " << bitmap.m_height << "," << (void*) bitmap.m_pixels << ","
            << (void*) bitmap.m_clut << "}";
    return stream;
}

inline std::ostream& operator <<(std::ostream& stream,
                                 const MethodData& methodData)
{
    stream << "MethodData::";
    switch (methodData.method)
    {
        case MethodData::Method::getStc:
            stream << "gfxStc";
            break;
        case MethodData::Method::gfxSetDisplayBounds:
            stream << "gfxSetDisplayBounds displayBounds: " << methodData.gfxSetDisplayBoundsArgs.displayBounds
                    << " windowBounds: " << methodData.gfxSetDisplayBoundsArgs.windowBounds;
            break;
        case MethodData::Method::gfxDraw:
            stream << "gfxDraw bitmap: " << methodData.gfxDrawArgs.bitmap << " srcRect: "
                    << methodData.gfxDrawArgs.srcRect << " dstRect: " << methodData.gfxDrawArgs.dstRect;
            break;
        case MethodData::Method::gfxClear:
            stream << "gfxClear rect: " << methodData.gfxClearArgs.rect;
            break;
        case MethodData::Method::gfxFinish:
            stream << "gfxFinish rect: " << methodData.gfxFinishArgs.rect;
            break;
        case MethodData::Method::gfxAllocate:
            stream << "gfxAllocate";
            break;
        case MethodData::Method::gfxFree:
            stream << "gfxFree";
            break;
        default:
            stream << "unknown: " << static_cast<int>(methodData.method);
            break;
    }
    return stream;
}

inline bool operator==(const Rectangle& lhs,
                       const Rectangle& rhs)
{
    return ((lhs.m_x1 == rhs.m_x1) && (lhs.m_y1 == rhs.m_y1) && (lhs.m_x2 == rhs.m_x2) && (lhs.m_y2 == rhs.m_y2));
}

inline bool operator!=(const Rectangle& lhs,
                       const Rectangle& rhs)
{
    return !(lhs == rhs);
}

inline bool operator==(const Bitmap& lhs,
                       const Bitmap& rhs)
{
    return ((lhs.m_height == rhs.m_height) && (lhs.m_width == rhs.m_width));
}

inline bool operator!=(const Bitmap& lhs,
                       const Bitmap& rhs)
{
    return !(lhs == rhs);
}

inline bool operator==(const MethodData& lhs,
                       const MethodData& rhs)
{
    std::cout << " lhs: " << lhs << " rhs: " << rhs << std::endl;
    if (lhs.method != rhs.method)
    {
        return false;
    }

    switch (lhs.method)
    {
        case MethodData::Method::gfxSetDisplayBounds:
            return (lhs.gfxSetDisplayBoundsArgs.displayBounds == rhs.gfxSetDisplayBoundsArgs.displayBounds)
                    && (lhs.gfxSetDisplayBoundsArgs.windowBounds == rhs.gfxSetDisplayBoundsArgs.windowBounds);
        case MethodData::Method::gfxDraw:
            return (lhs.gfxDrawArgs.bitmap == rhs.gfxDrawArgs.bitmap)
                    && (lhs.gfxDrawArgs.dstRect == rhs.gfxDrawArgs.dstRect)
                    && (lhs.gfxDrawArgs.srcRect == rhs.gfxDrawArgs.srcRect);
        case MethodData::Method::gfxClear:
            return (lhs.gfxClearArgs.rect == rhs.gfxClearArgs.rect);
        case MethodData::Method::gfxFinish:
            return (lhs.gfxFinishArgs.rect == rhs.gfxFinishArgs.rect);
        default:
            return true;
    }
}

inline bool operator!=(const MethodData& lhs,
                       const MethodData& rhs)
{
    return !(lhs == rhs);
}

class ClientCallHistory : public std::vector<MethodData>
{
public:

    ClientCallHistory()
    {
        // noop
    }

    ClientCallHistory(MethodData methodData)
    {
        push_back(methodData);
    }

    ClientCallHistory(std::initializer_list<MethodData> data) :
            std::vector<MethodData>(data)
    {
        // noop
    }

    bool operator!=(const ClientCallHistory& other) const
    {
        return (!(*this == other));
    }
};

class DecoderClientMock : public DecoderClient,
                          public TimeProvider
{
public:

    DecoderClientMock() :
            m_callbackHistory(),
            m_allocator(),
            m_stcTime()
    {
        // noop
    }

    const ClientCallHistory& getCallbackHistory()
    {
        return m_callbackHistory;
    }

    void clearCallbackHistory()
    {
        m_callbackHistory.clear();
    }

    void setStc(StcTime stcTime)
    {
        m_stcTime = stcTime;
    }

    /**
     * Returns STC time.
     *
     * @return
     *      STC time information.
     */
    virtual StcTime getStc() override
    {
        std::cout << __func__ << ": " << m_stcTime.m_time << std::endl;

        MethodData data;
        data.method = MethodData::Method::getStc;

        m_callbackHistory.push_back(data);
        return m_stcTime;
    }

    /**
     * Sets display and window sizes.
     *
     * Implementation should clear the entire screen.
     *
     * @param displayBounds
     *      Display rectangle.
     * @param windowBounds
     *      Window rectangle.
     */
    virtual void gfxSetDisplayBounds(const Rectangle& displayBounds,
                                     const Rectangle& windowBounds) override
    {
        std::cout << __func__ << " disp: " << displayBounds << " window: " << windowBounds << std::endl;

        MethodData data;
        data.method = MethodData::Method::gfxSetDisplayBounds;
        data.gfxSetDisplayBoundsArgs.displayBounds = displayBounds;
        data.gfxSetDisplayBoundsArgs.windowBounds = windowBounds;

        m_callbackHistory.push_back(data);
    }

    /**
     * Draws bitmap.
     *
     * @param bitmap
     *      Bitmap to draw.
     * @param srcRect
     *      Source rectangle (bitmap section to draw).
     * @param dstRect
     *      Destination rectangle (in display coordinates).
     */
    virtual void gfxDraw(const Bitmap& bitmap,
                         const Rectangle& srcRect,
                         const Rectangle& dstRect) override
    {
        std::cout << __func__ << " srcRect: " << srcRect << " dst: " << dstRect << " bitmap: " << bitmap << std::endl;

        MethodData data;
        data.method = MethodData::Method::gfxDraw;
        data.gfxDrawArgs.bitmap = bitmap;
        data.gfxDrawArgs.srcRect = srcRect;
        data.gfxDrawArgs.dstRect = dstRect;

        m_callbackHistory.push_back(data);
    }

    /**
     * Clears rectangle.
     *
     * @param rect
     *      Rectangle to clear (in display coordinates).
     */
    virtual void gfxClear(const Rectangle& rect) override
    {
        std::cout << __func__ << " rect: " << rect << std::endl;

        MethodData data;
        data.method = MethodData::Method::gfxClear;
        data.gfxClearArgs.rect = rect;

        m_callbackHistory.push_back(data);
    }

    /**
     * Finishes drawing operations.
     *
     * @param rect
     *      Rectangle modified by drawing operations (in display coordinates).
     */
    virtual void gfxFinish(const Rectangle& rect) override
    {
        std::cout << __func__ << " rect: " << rect << std::endl;

        MethodData data;
        data.method = MethodData::Method::gfxFinish;
        data.gfxFinishArgs.rect = rect;

        m_callbackHistory.push_back(data);
    }

    /**
     * Allocates graphics memory for bitmaps.
     *
     * @param size
     *      Size of the block to allocate in bytes.
     *
     * @return
     *      Allocated block or null if memory is not available.
     */
    virtual void* gfxAllocate(std::uint32_t size) override
    {
        void* block = m_allocator.allocate(size, 1);

        std::cout << __func__ << " size: " << size << " block: " << (void*) block << std::endl;

        MethodData data;
        data.method = MethodData::Method::gfxAllocate;

        m_callbackHistory.push_back(data);

        return block;
    }

    /**
     * Releases graphics memory.
     *
     * @param block
     *      Block of memory allocated using gfxAllocate().
     */
    virtual void gfxFree(void* block) override
    {
        std::cout << __func__ << " block: " << (void*) block << std::endl;
        m_allocator.free(block);

        MethodData data;
        data.method = MethodData::Method::gfxFree;

        m_callbackHistory.push_back(data);
    }

private:

    DynamicAllocator m_allocator;
    ClientCallHistory m_callbackHistory;

    StcTime m_stcTime;
};

#endif // DVBSUBDECODER_TEST_DECODERCLIENT_MOCK_HPP

