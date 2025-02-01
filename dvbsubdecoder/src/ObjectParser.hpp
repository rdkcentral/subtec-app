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


#ifndef DVBSUBDECODER_OBJECTPARSER_HPP_
#define DVBSUBDECODER_OBJECTPARSER_HPP_

#include <cstdint>
#include <array>

namespace dvbsubdecoder
{

class PesPacketReader;
class PixelWriter;

/**
 * Objects data parser.
 */
class ObjectParser
{
public:
    /**
     * Constructor.
     *
     * @param reader
     *      Object data reader.
     * @param writer
     *      Region pixels writer.
     */
    ObjectParser(PesPacketReader& reader,
                 PixelWriter& writer);

    /**
     * Parses the object.
     *
     * The data is read from reader given in constructor and the
     * decoded pixels are stored using writer given in constructor.
     */
    void parse();

private:
    /**
     * Parses 2-bit pixel code string.
     */
    void parse2bitPixelCodeString();

    /**
     * Parses 4-bit pixel code string.
     */
    void parse4bitPixelCodeString();

    /**
     * Parses 8-bit pixel code string.
     */
    void parse8bitPixelCodeString();

    /**
     * Parses 2-to-4-bit map table.
     */
    void parseMapTable2to4();

    /**
     * Parses 2-to-8-bit map table.
     */
    void parseMapTable2to8();

    /**
     * Parses 4-to-8-bit map table.
     */
    void parseMapTable4to8();

    /**
     * Sets pixels.
     *
     * @param pixelCode
     *      Pixel code (CLUT index).
     * @param count
     *      Number of pixels to set.
     */
    void setPixels(std::uint8_t pixelCode,
                   std::uint32_t count);

    /** Object data reader. */
    PesPacketReader& m_reader;

    /** Region pixels writer. */
    PixelWriter& m_writer;

    /** Color map table. */
    std::array<uint8_t,4> m_mapTable2to4;

    /** Color map table. */
    std::array<uint8_t,4> m_mapTable2to8;

    /** Color map table. */
    std::array<uint8_t,16> m_mapTable4to8;

    /**
     * Current map for CLUT indexes.
     *
     * Null if no mapping is in use.
     */
    std::uint8_t* m_currentMap;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_OBJECTPARSER_HPP_*/
