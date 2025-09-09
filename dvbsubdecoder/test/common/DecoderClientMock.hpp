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


#ifndef DVBSUBDECODER_DECODERCLIENTMOCK_HPP_
#define DVBSUBDECODER_DECODERCLIENTMOCK_HPP_

#include "DecoderClient.hpp"
#include "TimeProvider.hpp"

class DecoderClientMock : public dvbsubdecoder::DecoderClient,
                          public dvbsubdecoder::TimeProvider
{
public:
    using StcTime = dvbsubdecoder::StcTime;
    using Rectangle = dvbsubdecoder::Rectangle;
    using Bitmap = dvbsubdecoder::Bitmap;

    DecoderClientMock() :
            m_allocLimit(0),
            m_allocTotal(0),
            m_nextStc()
    {
        // noop
    }

    virtual StcTime getStc() override
    {
        return m_nextStc;
    }

    virtual void gfxSetDisplayBounds(const Rectangle& displayBounds,
                                     const Rectangle& windowBounds) override
    {
        // noop
    }

    virtual void gfxDraw(const Bitmap& bitmap,
                         const Rectangle& srcRect,
                         const Rectangle& dstRect) override
    {
        // noop
    }

    virtual void gfxClear(const Rectangle& rect) override
    {
        // noop
    }

    virtual void gfxFinish(const Rectangle& rect) override
    {
        // noop
    }

    virtual void* gfxAllocate(std::uint32_t size) override
    {
        if (m_allocLimit < m_allocTotal)
        {
            return nullptr;
        }

        auto bytesLeft = m_allocLimit - m_allocTotal;
        if (bytesLeft < size)
        {
            return nullptr;
        }

        void* block = ::operator new(size);

        m_allocBlocks.insert(std::make_pair(block, size));
        m_allocTotal += size;

        return block;
    }

    virtual void gfxFree(void* block) override
    {
        if (block)
        {
            auto iter = m_allocBlocks.find(block);
            if (iter != m_allocBlocks.end())
            {
                m_allocTotal -= iter->second;
                m_allocBlocks.erase(iter);
            }
            else
            {
                throw std::logic_error("Unknown block");
            }
        }
    }

    void setAllocLimit(std::size_t allocLimit)
    {
        m_allocLimit = allocLimit;
    }

    void setNextStc(const StcTime& nextStc)
    {
        m_nextStc = nextStc;
    }

    std::size_t getAllocTotal() const
    {
        return m_allocTotal;
    }

private:
    std::size_t m_allocTotal;
    std::size_t m_allocLimit;
    std::map<void*,size_t> m_allocBlocks;
    StcTime m_nextStc;
};

#endif /*DVBSUBDECODER_DECODERCLIENTMOCK_HPP_*/
