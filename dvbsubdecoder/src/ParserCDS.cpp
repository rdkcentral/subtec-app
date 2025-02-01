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


#include "ParserCDS.hpp"

#include <subttxrend/common/Logger.hpp>

#include "PesPacketReader.hpp"
#include "Database.hpp"
#include "Consts.hpp"
#include "ColorCalculator.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "ParserCDS");

} // namespace <anonmymous>

void ParserCDS::parseClutDefinitionSegment(Database& database,
                                           PesPacketReader& reader)
{
    g_logger.trace("%s", __func__);

    if (database.getPage().getState() != Page::State::INCOMPLETE)
    {
        g_logger.trace("%s - page not currently being parsed", __func__);
        return;
    }

    // == parse header ==
    const std::uint8_t clutId = reader.readUint8();
    const std::uint8_t versionFlags = reader.readUint8();
    const std::uint8_t versionNumber = (versionFlags >> 4) & 0x0F;

    g_logger.trace("%s - id: %02X", __func__, clutId);
    g_logger.trace("%s - version: %X", __func__, versionNumber);

    // == find and check version ==

    auto clut = database.getClutById(clutId);
    if (!clut)
    {
        g_logger.info("%s - clut %u not found, skipping", __func__, clutId);
        return;
    }

    if (clut->getVersion() == versionNumber)
    {
        g_logger.trace("%s - same version, skipping", __func__);
        return;
    }

    // == parse data ==

    while (reader.getBytesLeft() > 0)
    {
        const std::uint8_t entryId = reader.readUint8();
        const std::uint8_t entryFlags = reader.readUint8();

        g_logger.trace("%s - entry %u - flags %u", __func__, entryId,
                entryFlags);

        ColorYCrCbT ycrcbt;

        if ((entryFlags & ClutFlagsBits::FULL_RANGE) != 0)
        {
            ycrcbt.m_y = reader.readUint8();
            ycrcbt.m_cr = reader.readUint8();
            ycrcbt.m_cb = reader.readUint8();
            ycrcbt.m_t = reader.readUint8();

            g_logger.trace("%s - entry %u - full - Y=%u Cr=%u Cb=%u T=%u",
                    __func__, entryId, ycrcbt.m_y, ycrcbt.m_cr, ycrcbt.m_cb,
                    ycrcbt.m_t);
        }
        else
        {
            const std::uint16_t value = reader.readUint16be();

            ycrcbt.m_y = (value >> 10) & 0x3F;
            ycrcbt.m_cr = (value >> 6) & 0x0F;
            ycrcbt.m_cb = (value >> 2) & 0x0F;
            ycrcbt.m_t = (value >> 0) & 0x03;

            g_logger.trace("%s - entry %u - small - Y=%u Cr=%u Cb=%u T=%u",
                    __func__, entryId, ycrcbt.m_y, ycrcbt.m_cr, ycrcbt.m_cb,
                    ycrcbt.m_t);

            // convert to full range
            ycrcbt.m_y <<= 2;
            ycrcbt.m_cr <<= 4;
            ycrcbt.m_cb <<= 4;
            ycrcbt.m_t <<= 6;

            g_logger.trace("%s - entry %u - extended - Y=%u Cr=%u Cb=%u T=%u",
                    __func__, entryId, ycrcbt.m_y, ycrcbt.m_cr, ycrcbt.m_cb,
                    ycrcbt.m_t);

        }

        auto argb = ColorCalculator().toARGB(ycrcbt);
        auto argbValue = argb.toUint32();

        g_logger.trace("%s - entry %u - ARGB=%08X", __func__, entryId,
                argbValue);

        if ((entryFlags & ClutFlagsBits::ENTRY_2BIT) != 0)
        {
            if (entryId < (1 << 2))
            {
                g_logger.trace("%s - entry 2bit[%u] set to ARGB=%08X", __func__,
                        entryId, argbValue);

                clut->set2bit(entryId, argbValue);
            }
            else
            {
                g_logger.warning("%s - entry %u outside 2 bit range", __func__,
                        entryId);
            }
        }
        if ((entryFlags & ClutFlagsBits::ENTRY_4BIT) != 0)
        {
            if (entryId < (1 << 4))
            {
                g_logger.trace("%s - entry 4bit[%u] set to ARGB=%08X", __func__,
                        entryId, argbValue);

                clut->set4bit(entryId, argbValue);
            }
            else
            {
                g_logger.warning("%s - entry %u outside 4 bit range", __func__,
                        entryId);
            }
        }
        if ((entryFlags & ClutFlagsBits::ENTRY_8BIT) != 0)
        {
            g_logger.trace("%s - entry 8bit[%u] set to ARGB=%08X", __func__,
                    entryId, argbValue);

            clut->set8bit(entryId, argbValue);
        }
    }

    // == finish ==

    clut->setVersion(versionNumber);
}

} // namespace dvbsubdecoder
