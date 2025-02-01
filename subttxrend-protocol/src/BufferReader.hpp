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

#pragma once

#include "Buffer.hpp"

namespace subttxrend
{
namespace protocol
{

/**
 * Reader for the buffer contents.
 *
 * The reader automatically shifts current offset when extracting values.
 */
class BufferReader
{
public:
    /**
     * Constructor.
     *
     * @param buffer
     *      Buffer with data.
     */
    BufferReader(const Buffer& buffer);

    /**
     * Destructor.
     */
    virtual ~BufferReader() = default;

    /**
     * Returns buffer for which reader was made.
     *
     * @return
     *      Buffer.
     */
    const Buffer& getBuffer() const;

    /**
     * Returns size of the data.
     *
     * @return
     *      Size of the data in bytes.
     */
    std::size_t getSize() const;

    /**
     * Returns current offset.
     *
     * @return
     *      Current offset in bytes.
     */
    std::size_t getOffset() const;

    /**
     * Extracts value from data.
     *
     * @param value
     *      Variable to store the value.
     *
     * @retval true
     *      Value was successfully extracted.
     * @retval false
     *      Extraction failed.
     */
    bool extractLeUint32(std::uint32_t& value);

    /**
     * Extracts value from data.
     *
     * @param value
     *      Variable to store the value.
     *
     * @retval true
     *      Value was successfully extracted.
     * @retval false
     *      Extraction failed.
     */
    bool extractLeUint64(std::uint64_t& value);

    /**
     * Extracts value from data.
     *
     * @param value
     *      Variable to store the value.
     *
     * @retval true
     *      Value was successfully extracted.
     * @retval false
     *      Extraction failed.
     */
    bool extractLeInt64(std::int64_t& value);

    /**
     * Extract buffer from data.
     *
     * @param wantedSize
     *      Wanted size of the buffer.
     * @param buffer
     *      Variable to store buffer.
     *
     * @retval true
     *      Value was successfully extracted.
     * @retval false
     *      Extraction failed.
     */
    bool extractBuffer(std::size_t wantedSize,
                       std::vector<char>& buffer);

private:
    /** Buffer with data. */
    const Buffer& m_buffer;

    /** Current offset. */
    std::size_t m_offset;
};

} // namespace protocol
} // namespace subttxrend

