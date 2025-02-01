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


#include "Clut.hpp"

#include <array>
#include <subttxrend/common/Logger.hpp>

#include "Consts.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "Clut");

/** CLUT - 2-bit depth. */
std::array<std::uint32_t, 1 << 2> g_defaultClut2bit;

/** CLUT - 4-bit depth. */
std::array<std::uint32_t, 1 << 4> g_defaultClut4bit;

/** CLUT - 8-bit depth. */
std::array<std::uint32_t, 1 << 8> g_defaultClut8bit;

/** Flag indicating if default CLUTs were initialized. */
bool g_defaultClutInited = false;

/**
 * ARGB value in percents.
 * @{
 */
const std::uint8_t p100 = 0xFF;
const std::uint8_t p75 = 0xBF;
const std::uint8_t p67 = 0xAA;
const std::uint8_t p50 = 0x7F;
const std::uint8_t p33 = 0x55;
const std::uint8_t p17 = 0x2B;
const std::uint8_t p0 = 0x00;
/** @} */

/**
 * Create ARGB value from components.
 *
 * @param r
 *      Component value.
 * @param g
 *      Component value.
 * @param b
 *      Component value.
 * @param t
 *      Component value.
 *
 * @return
 * ARGB value (A on most significant bits).
 */
std::uint32_t makeARGB(std::uint8_t r,
                       std::uint8_t g,
                       std::uint8_t b,
                       std::uint8_t t)
{
    std::uint32_t argb = 0;

    argb |= static_cast<std::uint32_t>(255 - t) << 24;
    argb |= static_cast<std::uint32_t>(r) << 16;
    argb |= static_cast<std::uint32_t>(g) << 8;
    argb |= static_cast<std::uint32_t>(b) << 0;

    return argb;
}

/**
 * Initializes default CLUT table - 2 bit.
 */
void initDefaultClut2bit()
{
    const std::size_t count = (1 << 2);
    for (std::size_t i = 0; i < count; ++i)
    {
        uint8_t r = p0;
        uint8_t g = p0;
        uint8_t b = p0;
        uint8_t t = p0;

        bool b1 = (i & (1 << 1)) != 0;
        bool b2 = (i & (1 << 0)) != 0;

        if ((b1 == false) && (b2 == false))
        {
            t = p100;
        }
        if ((b1 == false) && (b2 == true))
        {
            r = g = b = p100;
            t = p0;
        }
        if ((b1 == true) && (b2 == false))
        {
            r = g = b = p0;
            t = p0;
        }
        if ((b1 == true) && (b2 == true))
        {
            r = g = b = p50;
            t = p0;
        }

        g_defaultClut2bit[i] = makeARGB(r, g, b, t);

        g_logger.trace("%s - entry: %03zu = %08X", __func__, i,
                g_defaultClut2bit[i]);
    }
}

/**
 * Initializes default CLUT table - 4 bit.
 */
void initDefaultClut4bit()
{
    const std::size_t count = (1 << 4);
    for (std::size_t i = 0; i < count; ++i)
    {
        uint8_t r = p0;
        uint8_t g = p0;
        uint8_t b = p0;
        uint8_t t = p0;

        bool b1 = (i & (1 << 3)) != 0;
        bool b2 = (i & (1 << 2)) != 0;
        bool b3 = (i & (1 << 1)) != 0;
        bool b4 = (i & (1 << 0)) != 0;

        if (b1 == false)
        {
            if ((b2 == false) && (b3 == false) && (b4 == false))
            {
                t = p100;
            }
            else
            {
                r = b4 ? p100 : p0;
                g = b3 ? p100 : p0;
                b = b2 ? p100 : p0;
                t = p0;
            }
        }
        else
        {
            r = b4 ? p50 : p0;
            g = b3 ? p50 : p0;
            b = b2 ? p50 : p0;
            t = p0;
        }

        g_defaultClut4bit[i] = makeARGB(r, g, b, t);

        g_logger.trace("%s - entry: %03zu = %08X", __func__, i,
                g_defaultClut4bit[i]);
    }
}

/**
 * Initializes default CLUT table - 8 bit.
 */
void initDefaultClut8bit()
{
    const std::size_t count = (1 << 8);
    for (std::size_t i = 0; i < count; ++i)
    {
        uint8_t r = p0;
        uint8_t g = p0;
        uint8_t b = p0;
        uint8_t t = p0;

        bool b1 = (i & (1 << 7)) != 0;
        bool b2 = (i & (1 << 6)) != 0;
        bool b3 = (i & (1 << 5)) != 0;
        bool b4 = (i & (1 << 4)) != 0;
        bool b5 = (i & (1 << 3)) != 0;
        bool b6 = (i & (1 << 2)) != 0;
        bool b7 = (i & (1 << 1)) != 0;
        bool b8 = (i & (1 << 0)) != 0;

        if ((b1 == false) && (b5 == false))
        {
            if ((b2 == false) && (b3 == false) && (b4 == false))
            {
                if ((b6 == false) && (b7 == false) && (b8 == false))
                {
                    t = p100;
                }
                else
                {
                    r = b8 ? p100 : p0;
                    g = b7 ? p100 : p0;
                    b = b6 ? p100 : p0;
                    t = p75;
                }
            }
            else
            {
                r = (b8 ? p33 : p0) + (b4 ? p67 : p0);
                g = (b7 ? p33 : p0) + (b3 ? p67 : p0);
                b = (b6 ? p33 : p0) + (b2 ? p67 : p0);
                t = p0;
            }
        }
        else if ((b1 == false) && (b5 == true))
        {
            r = (b8 ? p33 : p0) + (b4 ? p67 : p0);
            g = (b7 ? p33 : p0) + (b3 ? p67 : p0);
            b = (b6 ? p33 : p0) + (b2 ? p67 : p0);
            t = p50;
        }
        else if ((b1 == true) && (b5 == false))
        {
            r = (b8 ? p17 : p0) + (b4 ? p33 : p0) + p50;
            g = (b7 ? p17 : p0) + (b3 ? p33 : p0) + p50;
            b = (b6 ? p17 : p0) + (b2 ? p33 : p0) + p50;
            t = p0;
        }
        else if ((b1 == true) && (b5 == true))
        {
            r = (b8 ? p17 : p0) + (b4 ? p33 : p0);
            g = (b7 ? p17 : p0) + (b3 ? p33 : p0);
            b = (b6 ? p17 : p0) + (b2 ? p33 : p0);
            t = p0;
        }

        g_defaultClut8bit[i] = makeARGB(r, g, b, t);

        g_logger.trace("%s - entry: %03zu = %08X", __func__, i,
                g_defaultClut8bit[i]);
    }
}

/**
 * Initializes default CLUT tables.
 */
void initDefaultCluts()
{
    initDefaultClut2bit();
    initDefaultClut4bit();
    initDefaultClut8bit();
}

} // namespace <anonmymous>

Clut::Clut()
{
    reset();
}

void Clut::reset()
{
    m_version = INVALID_VERSION;

    if (!g_defaultClutInited)
    {
        initDefaultCluts();
        g_defaultClutInited = true;
    }

    m_clut2bit = g_defaultClut2bit;
    m_clut4bit = g_defaultClut4bit;
    m_clut8bit = g_defaultClut8bit;
}

} // namespace dvbsubdecoder
