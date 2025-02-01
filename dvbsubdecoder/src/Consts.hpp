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


#ifndef DVBSUBDECODER_CONSTS_HPP_
#define DVBSUBDECODER_CONSTS_HPP_

#include <cstdint>

namespace dvbsubdecoder
{

/**
 * Invalid version value.
 *
 * Version is 4-bit field so this value must be outside 0x00..0x0F range.
 */
const std::uint8_t INVALID_VERSION = 0xFF;

/**
 * Subtitles PES bits values.
 */
namespace PesBits
{

/** Subtitles PES constants - Data identifier. */
const std::uint8_t SUBTITLE_DATA_IDENTIFIER = 0x20;

/** Subtitles PES constants - Stream ID. */
const std::uint8_t STREAM_ID_VALUE = 0x00;

/** Subtitles PES constants - Sync byte. */
const std::uint8_t SYNC_BYTE_VALUE = 0x0F;

/** Subtitles PES constants - End marker. */
const std::uint8_t END_MARKER_VALUE = 0xFF;

} // namespace PesBits

/**
 * Region color depth & compatibility bits.
 */
namespace RegionDepthBits
{

/** Depth code - 2-bit. */
const std::uint8_t DEPTH_2BIT = 0x01;
/** Depth code - 4-bit. */
const std::uint8_t DEPTH_4BIT = 0x02;
/** Depth code - 8-bit. */
const std::uint8_t DEPTH_8BIT = 0x03;

} // namespace RegionDepthBits

/**
 * Bits values - Page state.
 */
namespace PageStateBits
{

/** PCS - page state - normal case. */
const std::uint8_t NORMAL_CASE = 0x00;
/** PCS - page state - acquisition point. */
const std::uint8_t ACQUISITION_POINT = 0x01;
/** PCS - page state - mode change. */
const std::uint8_t MODE_CHANGE = 0x02;
/** PCS - page state - reserved. */
const std::uint8_t RESERVED = 0x03;

} // namespace PageStateBits

/**
 * Bits values - Object coding method.
 */
namespace ObjectCodingMethodBits
{

/** Coding method - pixels. */
const std::uint8_t PIXELS = 0x00;

} // namespace ObjectCodingMethodBits

/**
 * Bits values - CLUT flags.
 */
namespace ClutFlagsBits
{

/** Entry for 2-bit CLUT. */
const std::uint8_t ENTRY_2BIT = (1 << 7);
/** Entry for 4-bit CLUT. */
const std::uint8_t ENTRY_4BIT = (1 << 6);
/** Entry for 8-bit CLUT. */
const std::uint8_t ENTRY_8BIT = (1 << 5);
/** CLUT entry defined in full range. */
const std::uint8_t FULL_RANGE = (1 << 0);

} // namespace ClutFlagsBits

/**
 * Segment type.
 */
namespace SegmentTypeBits
{

/** Segment type constants. */
const std::uint8_t PAGE_COMPOSITION = 0x10;

/** Segment type constants. */
const std::uint8_t REGION_COMPOSITION = 0x11;

/** Segment type constants. */
const std::uint8_t CLUT_DEFINITION = 0x12;

/** Segment type constants. */
const std::uint8_t OBJECT_DATA = 0x13;

/** Segment type constants. */
const std::uint8_t DISPLAY_DEFINITION = 0x14;

/** Segment type constants. */
const std::uint8_t DISPARITY_SIGNALLING = 0x15;

/** Segment type constants. */
const std::uint8_t END_OF_DISPLAY_SET = 0x80;

} // namespace SegmentTypeBits

/**
 * Region object type.
 */
namespace RegionObjectTypeBits
{

/** Region object type - Bitmap. */
const std::uint8_t BASIC_BITMAP = 0x00;
/** Region object type - Basic character. */
const std::uint8_t BASIC_CHARACTER = 0x01;
/** Region object type - Composite string. */
const std::uint8_t COMPOSITE_STRING = 0x02;

} // namespace RegionObjectTypeBits

/**
 * Object provider bits.
 */
namespace RegionObjectProviderBits
{

/** Object provider - subtitling stream. */
const std::uint8_t SUBTITLING_STREAM = 0x00;

} // namespace RegionObjectProviderBits

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_CONSTS_HPP_*/
