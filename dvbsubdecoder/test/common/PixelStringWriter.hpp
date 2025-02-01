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


#ifndef DVBSUBDECODER_PIXELSTRINGWRITER_HPP_
#define DVBSUBDECODER_PIXELSTRINGWRITER_HPP_

#include "BitStreamWriter.hpp"

/**
 * Utility for writing pixel strings.
 */
class PixelStringWriter
{
public:
    /**
     * Constructor.
     *
     * Creates empty pixel string.
     */
    PixelStringWriter()
    {
        clear();
    }

    /**
     * Clears the pixel string contents.
     */
    void clear()
    {
        m_currentDataType = -1;
        m_bitStreamWriter.clear();
    }

    /**
     * Returns pixel string bytes.
     *
     * @return
     *      Byte array with pixel string.
     */
    const std::uint8_t* data() const
    {
        return m_bitStreamWriter.data();
    }

    /**
     * Returns pixel string bytes size.
     *
     * @return
     *      Size of pixel string in bytes.
     */
    const std::size_t size() const
    {
        return m_bitStreamWriter.size();
    }

    void writeEndOfLine()
    {
        startDataType(DATA_TYPE_END_OF_OBJECT_LINE);
        endDataType(DATA_TYPE_END_OF_OBJECT_LINE);
    }

    void start2bitPixelCodeString()
    {
        startDataType(DATA_TYPE_PIXELS_2BIT);
    }

    void write2bitPixels(std::uint8_t colorIndex,
                         std::size_t count)
    {
        assert(m_currentDataType == DATA_TYPE_PIXELS_2BIT);
        assert(colorIndex < (1 << 2));

        if (count == 0)
        {
            return;
        }

        while (count > 284)
        {
            write2bitPixels(colorIndex, 284);
            count -= 284;
        }

        if (count == 1)
        {
            switch (colorIndex)
            {
            case 0:
                m_bitStreamWriter.write(1, 4);
                break;
            case 1:
                m_bitStreamWriter.write(1, 2);
                break;
            case 2:
                m_bitStreamWriter.write(2, 2);
                break;
            case 3:
                m_bitStreamWriter.write(3, 2);
                break;
            default:
                assert(false);
                break;
            }
        }
        else if (count == 2)
        {
            if (colorIndex == 0)
            {
                m_bitStreamWriter.write(1, 6);
            }
            else
            {
                write2bitPixels(colorIndex, 1);
                write2bitPixels(colorIndex, 1);
            }
        }
        else if ((count >= 3) && (count <= 10))
        {
            m_bitStreamWriter.write(1, 3);
            m_bitStreamWriter.write(count - 3, 3);
            m_bitStreamWriter.write(colorIndex, 2);
        }
        else if (count == 11)
        {
            write2bitPixels(colorIndex, 1);
            write2bitPixels(colorIndex, 10);
        }
        else if ((count >= 12) && (count <= 27))
        {
            m_bitStreamWriter.write(2, 6);
            m_bitStreamWriter.write(count - 12, 4);
            m_bitStreamWriter.write(colorIndex, 2);
        }
        else if (count == 28)
        {
            write2bitPixels(colorIndex, 1);
            write2bitPixels(colorIndex, 27);
        }
        else if ((count >= 29) && (count <= 284))
        {
            m_bitStreamWriter.write(3, 6);
            m_bitStreamWriter.write(count - 29, 8);
            m_bitStreamWriter.write(colorIndex, 2);
        }
        else
        {
            assert(false);
        }
    }

    void end2bitPixelCodeString()
    {
        assert(m_currentDataType == DATA_TYPE_PIXELS_2BIT);

        m_bitStreamWriter.write(0, 6);

        const auto stuffSize = m_bitStreamWriter.getStuffSizeNeeded();
        switch (stuffSize)
        {
        case 0:
            break;
        case 2:
        case 4:
        case 6:
            m_bitStreamWriter.write(0, stuffSize);
            break;
        default:
            assert(0);
            break;
        }
        endDataType(DATA_TYPE_PIXELS_2BIT);
    }

    void start4bitPixelCodeString()
    {
        startDataType(DATA_TYPE_PIXELS_4BIT);
    }

    void write4bitPixels(std::uint8_t colorIndex,
                         std::size_t count)
    {
        assert(m_currentDataType == DATA_TYPE_PIXELS_4BIT);
        assert(colorIndex < (1 << 4));

        if (count == 0)
        {
            return;
        }

        while (count > 280)
        {
            write4bitPixels(colorIndex, 280);
            count -= 280;
        }

        if (colorIndex == 0)
        {
            if (count == 1)
            {
                m_bitStreamWriter.write(0x0C, 8);
            }
            else if (count == 2)
            {
                m_bitStreamWriter.write(0x0D, 8);
            }
            else if ((count >= 3) && (count <= 9))
            {
                m_bitStreamWriter.write(0x0, 4);
                m_bitStreamWriter.write(count - 2, 4);
            }
            else if ((count >= 10) && (count <= 24))
            {
                m_bitStreamWriter.write(0x0, 4);
                m_bitStreamWriter.write(0xE, 4);
                m_bitStreamWriter.write(count - 9, 4);
                m_bitStreamWriter.write(colorIndex, 4);
            }
            else if ((count >= 25) && (count <= 280))
            {
                m_bitStreamWriter.write(0x0, 4);
                m_bitStreamWriter.write(0xF, 4);
                m_bitStreamWriter.write(count - 25, 8);
                m_bitStreamWriter.write(colorIndex, 4);
            }
            else
            {
                assert(false);
            }
        }
        else
        {
            if (count == 1)
            {
                m_bitStreamWriter.write(colorIndex, 4);
            }
            else if (count == 2)
            {
                m_bitStreamWriter.write(colorIndex, 4);
                m_bitStreamWriter.write(colorIndex, 4);
            }
            else if (count == 3)
            {
                m_bitStreamWriter.write(colorIndex, 4);
                m_bitStreamWriter.write(colorIndex, 4);
                m_bitStreamWriter.write(colorIndex, 4);
            }
            else if ((count >= 4) && (count <= 7))
            {
                m_bitStreamWriter.write(0x0, 4);
                m_bitStreamWriter.write(0x2, 2);
                m_bitStreamWriter.write(count - 4, 2);
                m_bitStreamWriter.write(colorIndex, 4);
            }
            else if (count == 8)
            {
                write4bitPixels(colorIndex, 1);
                write4bitPixels(colorIndex, 7);
            }
            else if ((count >= 9) && (count <= 24))
            {
                m_bitStreamWriter.write(0x0, 4);
                m_bitStreamWriter.write(0xE, 4);
                m_bitStreamWriter.write(count - 9, 4);
                m_bitStreamWriter.write(colorIndex, 4);
            }
            else if ((count >= 25) && (count <= 280))
            {
                m_bitStreamWriter.write(0x0, 4);
                m_bitStreamWriter.write(0xF, 4);
                m_bitStreamWriter.write(count - 25, 8);
                m_bitStreamWriter.write(colorIndex, 4);
            }
            else
            {
                assert(false);
            }
        }
    }

    void end4bitPixelCodeString()
    {
        assert(m_currentDataType == DATA_TYPE_PIXELS_4BIT);

        m_bitStreamWriter.write(0, 8);

        const auto stuffSize = m_bitStreamWriter.getStuffSizeNeeded();
        switch (stuffSize)
        {
        case 0:
            break;
        case 4:
            m_bitStreamWriter.write(0, stuffSize);
            break;
        default:
            assert(0);
            break;
        }
        endDataType(DATA_TYPE_PIXELS_4BIT);
    }

    void start8bitPixelCodeString()
    {
        startDataType(DATA_TYPE_PIXELS_8BIT);
    }

    void write8bitPixels(std::uint8_t colorIndex,
                         std::size_t count)
    {
        assert(m_currentDataType == DATA_TYPE_PIXELS_8BIT);

        if (count == 0)
        {
            return;
        }

        while (count > 127)
        {
            write8bitPixels(colorIndex, 127);
            count -= 127;
        }

        if (colorIndex == 0)
        {
            if ((count >= 1) && (count <= 127))
            {
                m_bitStreamWriter.write(0x00, 8);
                m_bitStreamWriter.write(0x0, 1);
                m_bitStreamWriter.write(count, 7);
            }
            else
            {
                assert(false);
            }
        }
        else
        {
            if (count == 1)
            {
                m_bitStreamWriter.write(colorIndex, 8);
            }
            else if (count == 2)
            {
                m_bitStreamWriter.write(colorIndex, 8);
                m_bitStreamWriter.write(colorIndex, 8);
            }
            else if ((count >= 3) && (count <= 127))
            {
                m_bitStreamWriter.write(0x00, 8);
                m_bitStreamWriter.write(0x1, 1);
                m_bitStreamWriter.write(count, 7);
                m_bitStreamWriter.write(colorIndex, 8);
            }
            else
            {
                assert(false);
            }
        }
    }

    void end8bitPixelCodeString()
    {
        assert(m_currentDataType == DATA_TYPE_PIXELS_8BIT);

        m_bitStreamWriter.write(0, 8);
        m_bitStreamWriter.write(0, 8);

        assert(m_bitStreamWriter.getStuffSizeNeeded() == 0);
        endDataType(DATA_TYPE_PIXELS_8BIT);
    }

    void start2to4bitMap()
    {
        startDataType(DATA_TYPE_MAP_2TO4);
    }

    void write2to4bitMapEntry(std::uint8_t entry)
    {
        assert(m_currentDataType == DATA_TYPE_MAP_2TO4);
        m_bitStreamWriter.write(entry, 4);
    }

    void end2to4bitMap()
    {
        endDataType(DATA_TYPE_MAP_2TO4);
    }

    void start2to8bitMap()
    {
        startDataType(DATA_TYPE_MAP_2TO8);
    }

    void write2to8bitMapEntry(std::uint8_t entry)
    {
        assert(m_currentDataType == DATA_TYPE_MAP_2TO8);
        m_bitStreamWriter.write(entry, 8);
    }

    void end2to8bitMap()
    {
        endDataType(DATA_TYPE_MAP_2TO8);
    }

    void start4to8bitMap()
    {
        startDataType(DATA_TYPE_MAP_4TO8);
    }

    void write4to8bitMapEntry(std::uint8_t entry)
    {
        assert(m_currentDataType == DATA_TYPE_MAP_4TO8);
        m_bitStreamWriter.write(entry, 8);
    }

    void end4to8bitMap()
    {
        endDataType(DATA_TYPE_MAP_4TO8);
    }

private:
    static const std::uint8_t DATA_TYPE_PIXELS_2BIT = 0x10;
    static const std::uint8_t DATA_TYPE_PIXELS_4BIT = 0x11;
    static const std::uint8_t DATA_TYPE_PIXELS_8BIT = 0x12;

    static const std::uint8_t DATA_TYPE_MAP_2TO4 = 0x20;
    static const std::uint8_t DATA_TYPE_MAP_2TO8 = 0x21;
    static const std::uint8_t DATA_TYPE_MAP_4TO8 = 0x22;

    static const std::uint8_t DATA_TYPE_END_OF_OBJECT_LINE = 0xF0;

    void startDataType(std::uint8_t dataType)
    {
        assert(m_currentDataType == -1);

        assert(m_bitStreamWriter.getStuffSizeNeeded() == 0);
        m_bitStreamWriter.write(dataType, 8);

        m_currentDataType = dataType;
    }

    void endDataType(std::uint8_t dataType)
    {
        assert(dataType == m_currentDataType);
        assert(m_bitStreamWriter.getStuffSizeNeeded() == 0);

        m_currentDataType = -1;
    }

    /** Internal bit stream writer. */
    BitStreamWriter m_bitStreamWriter;

    /** Current data type. */
    std::int32_t m_currentDataType;
};

#endif /*DVBSUBDECODER_PIXELSTRINGWRITER_HPP_*/
