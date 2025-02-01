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


#ifndef DVBSUBDECODER_TEST_PESPACKETS_PESBUILDER_HPP
#define DVBSUBDECODER_TEST_PESPACKETS_PESBUILDER_HPP

#include <cassert>
#include <cstdint>
#include <cstdio>

#include <iostream>

#include "PacketBuffer.hpp"
#include "BitStreamWriter.hpp"

class SubtitlingSegment : public PacketBuffer
{
public:

    SubtitlingSegment(std::uint8_t segmentType,
                      std::uint16_t pageId)
    {
        append(SYNC_BYTE);
        append(segmentType);
        append(pageId);
    }

    virtual ~SubtitlingSegment() = 0;

protected:
    static constexpr std::uint8_t MAX_VERSION = 15;
    static constexpr std::size_t SEGMENT_SIZE_FIELD_LENGTH = 2;

private:
    static constexpr std::uint8_t SYNC_BYTE = 0x0F;
};

class PesBuilder : public PacketBuffer
{
public:

    PesBuilder() :
            m_pts(),
            m_finished(false)
    {
        // noop
    }

    void init(std::uint32_t pts)
    {
        m_pts = pts;

        std::cout << __func__ << " pts: " << m_pts << " " << pts << std::endl;
        appendHeader();
        appendOptionalHeader();
        appendPTS();
        appendPESDataField();
    }

    ~PesBuilder() = default;

    std::size_t getSize() const override
    {
        assert(m_finished);
        return PacketBuffer::getSize();
    }

    const std::uint8_t* getData() const override
    {
        assert(m_finished);
        return PacketBuffer::getData();
    }

    void addSegment(const SubtitlingSegment& segment)
    {
        assert(!m_finished);
        append(segment);
    }

    void finalize()
    {
        append(END_OF_PES);

        const std::uint16_t pesLength = m_data.size() - HEADER_SIZE;

        m_data[4] = HIGH_BYTE(pesLength);
        m_data[5] = LOW_BYTE(pesLength);

        m_finished = true;

        std::cout << __func__ << " getSize: " << getSize() << std::endl;
    }

private:

    void appendHeader()
    {
        append(static_cast<std::uint8_t>(0x00));
        append(static_cast<std::uint8_t>(0x00));
        append(static_cast<std::uint8_t>(0x01));
        append(STREAM_ID);

        // reserve space for pes length
        append(static_cast<std::uint16_t>(0x0000));
    }

    void appendOptionalHeader()
    {
        append(OPTIONAL_HEADER_BYTE1);
        append(OPTIONAL_HEADER_BYTE2);
        append(OPTIONAL_HEADER_BYTE3);
    }

    void appendPTS()
    {
        BitStreamWriter bitStream;

        bitStream.write(0x2, 4);
        bitStream.write(0x0, 1); // 33rd bit of STC
        bitStream.write(m_pts >> 29, 2);
        bitStream.write(0x1, 1);

        bitStream.write(((m_pts >> 15)), 15);
        bitStream.write(0x1, 1);

        bitStream.write(m_pts, 15);
        bitStream.write(0x1, 1);

        auto dataSize = bitStream.size();
        assert(dataSize == 5);

        append(bitStream.data(), dataSize);
    }

    void appendPESDataField()
    {
        append(SUBTITLE_DATA_IDENTIFIER);
        append(SUBTITLE_STREAM_ID);
    }

    static constexpr std::size_t HEADER_SIZE = 6;

    static constexpr std::uint8_t OPTIONAL_HEADER_BYTE1 = 0x85; // 10000101;
    static constexpr std::uint8_t OPTIONAL_HEADER_BYTE2 = 0x80; // 10000000;
    static constexpr std::uint8_t OPTIONAL_HEADER_BYTE3 = 0x05; // 10000000;

    static constexpr std::uint8_t STREAM_ID = 0xBD;
    static constexpr std::uint8_t SUBTITLE_DATA_IDENTIFIER = 0x20;
    static constexpr std::uint8_t SUBTITLE_STREAM_ID = 0x00;

    static constexpr std::uint8_t END_OF_PES = 0xFF;

    bool m_finished;
    std::uint32_t m_pts;

};

inline SubtitlingSegment::~SubtitlingSegment()
{
    // noop
}

class DataDefinitionSegment : public SubtitlingSegment
{
public:
    DataDefinitionSegment(std::uint16_t pageId,
                          std::uint8_t versionNumber,
                          bool displayWindowFlag,
                          std::uint16_t displayWidth,
                          std::uint16_t displayHeight,
                          std::uint16_t horizontalMin = 0,
                          std::uint16_t horizontalMax = 0,
                          std::uint16_t verticalMin = 0,
                          std::uint16_t verticalMax = 0) :
            SubtitlingSegment(DDS_SEGMENT_TYPE, pageId)
    {
        assert(versionNumber <= MAX_VERSION);
        assert(displayWidth <= MAX_WIDTH);
        assert(displayHeight <= MAX_HEIGHT);

        const auto sizeBefore = getSize();

        const std::uint16_t segmentSize = (displayWindowFlag ? SEGMENT_LENGTH_WITH_WINDOW : SEGMENT_LENGTH);
        append(segmentSize);

        const std::uint8_t versionAndDataFlagByte = (versionNumber << 4) | (displayWindowFlag ? 0x08 : 0);
        append(versionAndDataFlagByte);

        append(displayWidth);
        append(displayHeight);

        if (displayWindowFlag)
        {
            append(horizontalMin);
            append(horizontalMax);
            append(verticalMin);
            append(verticalMax);
        }

        assert(getSize() == sizeBefore + SEGMENT_SIZE_FIELD_LENGTH + segmentSize);
    }

private:

    static constexpr std::uint8_t DDS_SEGMENT_TYPE = 0x14;

    static constexpr std::uint8_t MAX_VERSION = 15;
    static constexpr std::uint16_t MAX_WIDTH = 4095;
    static constexpr std::uint16_t MAX_HEIGHT = 4095;

    static constexpr std::uint16_t SEGMENT_LENGTH = 5;
    static constexpr std::uint16_t SEGMENT_LENGTH_WITH_WINDOW = 13;

};

class PageCompositionSegment : public SubtitlingSegment
{
public:

    struct Region
    {
        std::uint8_t m_regionId;
        std::uint16_t m_horizontalAddress;
        std::uint16_t m_verticalAddress;
    };

    PageCompositionSegment(std::uint16_t pageId,
                           std::uint8_t pageTimeout,
                           std::uint8_t versionNumber,
                           std::uint8_t pageState,
                           const std::vector<Region>& regions) :
            SubtitlingSegment(PCS_SEGMENT_TYPE, pageId)
    {
        assert(versionNumber <= MAX_VERSION);
        assert(pageState <= MAX_STATE);

        const auto sizeBefore = getSize();

        const std::uint16_t segmentSize = LENGTH_WITHOUT_REGIONS + (regions.size() * REGION_DATA_SIZE);
        append(segmentSize);

        append(pageTimeout);

        const std::uint8_t versionNumberPageStateByte = (versionNumber << 4) | (pageState << 2);
        append(versionNumberPageStateByte);

        for (auto region : regions)
        {
            append(region.m_regionId);
            append(static_cast<std::uint8_t>(0x0)); /* reserved byte*/
            append(region.m_horizontalAddress);
            append(region.m_verticalAddress);
        }

        assert(getSize() == sizeBefore + SEGMENT_SIZE_FIELD_LENGTH + segmentSize);
    }

private:

    static constexpr std::uint8_t PCS_SEGMENT_TYPE = 0x10;
    static constexpr std::uint8_t MAX_STATE = 0x3;
    static constexpr std::uint16_t LENGTH_WITHOUT_REGIONS = 2;
    static constexpr std::uint16_t REGION_DATA_SIZE = 6;

};

class RegionCompositionSegment : public SubtitlingSegment
{
public:

    struct Object
    {
        std::uint16_t m_objectId;
        std::uint16_t m_horizontalPosition;
        std::uint16_t m_verticalPosition;

        static constexpr std::uint16_t MAX_HORIZONTAL_POSITION = 4095;
        static constexpr std::uint16_t MAX_VERTICAL_POSITION = 4095;
    };

    RegionCompositionSegment(std::uint16_t pageId,
                             std::uint8_t regionId,
                             std::uint8_t versionNumber,
                             bool fillFlag,
                             std::uint16_t regionWidth,
                             std::uint16_t regionHeight,
                             std::uint8_t levelOfCompatibility,
                             std::uint8_t regionDepth,
                             std::uint8_t clutId,
                             std::uint8_t region8bPixelCode,
                             std::uint8_t region4bPixelCode,
                             std::uint8_t region2bPixelCode,
                             const std::vector<Object>& objects) :
            SubtitlingSegment(RCS_SEGMENT_TYPE, pageId)
    {
        assert(versionNumber <= MAX_VERSION);

        const auto sizeBefore = getSize();

        const std::uint16_t segmentSize = LENGTH_WITHOUT_OBJECTS + (objects.size() * OBJECT_DATA_SIZE);
        append(segmentSize);

        append(regionId);

        const std::uint8_t versionNumberFillFlagByte = (versionNumber << 4) | (fillFlag ? 0x8 : 0x0);
        append(versionNumberFillFlagByte);

        append(regionWidth);
        append(regionHeight);

        const std::uint8_t levelOfCompatibilityDepthByte = (levelOfCompatibility << 5) | (regionDepth << 2);
        append(levelOfCompatibilityDepthByte);

        append(clutId);
        append(region8bPixelCode);

        const std::uint8_t pixelCode42Byte = (region4bPixelCode << 4) | (region2bPixelCode << 2);
        append(pixelCode42Byte);

        for (auto object : objects)
        {
            assert(object.m_horizontalPosition <= Object::MAX_HORIZONTAL_POSITION);
            assert(object.m_verticalPosition <= Object::MAX_VERTICAL_POSITION);

            BitStreamWriter writer;

            std::cout << " adding object: " << object.m_objectId << " " << object.m_horizontalPosition << " "
                    << object.m_verticalPosition << " " << std::endl;

            writer.write(object.m_objectId, 16);
            writer.write(0x0, 2);      // object type
            writer.write(0x0, 2);      // object provider flag
            writer.write(object.m_horizontalPosition, 12);
            writer.write(0x0, 4);      // reserved
            writer.write(object.m_verticalPosition, 12);

            append(writer.data(), writer.size());
        }

        assert(getSize() == sizeBefore + SEGMENT_SIZE_FIELD_LENGTH + segmentSize);
    }

private:
    static constexpr std::uint8_t RCS_SEGMENT_TYPE = 0x11;
    static constexpr std::uint8_t MAX_LEVEL_OF_COMPATIBILITY = 0x3;
    static constexpr std::uint16_t LENGTH_WITHOUT_OBJECTS = 10;
    static constexpr std::uint16_t OBJECT_DATA_SIZE = 6;
};

class ObjectDataSegment : public SubtitlingSegment
{
public:

    ObjectDataSegment(std::uint16_t pageId,
                      std::uint16_t objectId,
                      std::uint8_t versionNumber,
                      std::uint8_t codingMethod,
                      bool nonModyfingFlag,
                      std::uint16_t topFieldLength,
                      std::uint16_t bottomFieldLength,
                      const std::uint8_t* topFieldPixelData,
                      const std::uint8_t* bottomFieldPixelData) :
            SubtitlingSegment(ODS_SEGMENT_TYPE, pageId)
    {
        assert(versionNumber <= MAX_VERSION);
        assert(codingMethod <= MAX_CODING_METHOD);

        const auto sizeBefore = getSize();

        const std::uint16_t dataLength = (topFieldLength + bottomFieldLength);
        const bool isDataWordAligned = (dataLength % 2 == 0);

        const std::uint16_t segmentSize = LENGTH_WITHOUT_PIXEL_DATA + dataLength + (isDataWordAligned ? 0 : 1);
        append(segmentSize);

        append(objectId);

        BitStreamWriter writer;

        writer.write(versionNumber, 4);
        writer.write(codingMethod, 2);
        writer.write(nonModyfingFlag ? 0x1 : 0x0, 1);
        writer.write(0x0, 1);       // reserved

        writer.write(topFieldLength, 16);
        writer.write(bottomFieldLength, 16);

        append(writer.data(), writer.size());

        if (topFieldLength > 0)
        {
            append(topFieldPixelData, static_cast<std::size_t>(topFieldLength));
        }

        if (bottomFieldLength > 0)
        {
            append(bottomFieldPixelData, static_cast<std::size_t>(bottomFieldLength));
        }

        if (!isDataWordAligned)
        {
            append(static_cast<std::uint8_t>(0x0));
        }

        assert(getSize() == sizeBefore + SEGMENT_SIZE_FIELD_LENGTH + segmentSize);
    }

private:
    static constexpr std::uint8_t ODS_SEGMENT_TYPE = 0x13;
    static constexpr std::uint16_t LENGTH_WITHOUT_PIXEL_DATA = 7;

    static constexpr std::uint16_t MAX_CODING_METHOD = 0x3;

};

class EndOfDisplaySetSegment : public SubtitlingSegment
{
public:
    EndOfDisplaySetSegment(std::uint16_t pageId) :
            SubtitlingSegment(EODS_SEGMENT_TYPE, pageId)
    {
        // segment length
        append(static_cast<std::uint16_t>(0x0));
    }
private:
    static constexpr std::uint8_t EODS_SEGMENT_TYPE = 0x80;

};

#endif /* DVBSUBDECODER_TEST_PESPACKETS_PESBUILDER_HPP */
