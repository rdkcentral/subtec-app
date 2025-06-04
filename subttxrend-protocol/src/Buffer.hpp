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

#include <subttxrend/common/NonCopyable.hpp>

#include <cstdint>
#include <cstddef>
#include <vector>

namespace subttxrend
{
namespace protocol
{

/**
 * Wrapper for buffer with chars.
 */
class Buffer : private common::NonCopyable
{
public:
    /**
     * Constructor.
     *
     * @param data
     *      Array with data.
     * @param size
     *      Size of the data in bytes.
     */
    Buffer(const char* data,
           std::size_t size);

    /**
     * Destructor.
     */
    virtual ~Buffer() = default;

    /**
     * Returns size of the data.
     *
     * @return
     *      Size of the data in bytes.
     */
    std::size_t getSize() const;

    /**
     * Extracts value from data.
     *
     * @param offset
     *      Offset in buffer (in bytes) where the value starts.
     * @param value
     *      Variable to store the value.
     *
     * @retval true
     *      Value was successfully extracted.
     * @retval false
     *      Extraction failed.
     */
    bool extractLeUint32(std::size_t offset,
                         std::uint32_t& value) const;

    /**
     * Extracts value from data.
     *
     * @param offset
     *      Offset in buffer (in bytes) where the value starts.
     * @param value
     *      Variable to store the value.
     *
     * @retval true
     *      Value was successfully extracted.
     * @retval false
     *      Extraction failed.
     */
    bool extractLeUint64(std::size_t offset,
                         std::uint64_t& value) const;

    /**
     * Extracts value from data.
     *
     * @param offset
     *      Offset in buffer (in bytes) where the value starts.
     * @param value
     *      Variable to store the value.
     *
     * @retval true
     *      Value was successfully extracted.
     * @retval false
     *      Extraction failed.
     */
    bool extractLeInt64(std::size_t offset,
                         std::int64_t& value) const;

    /**
     * Extract buffer from data.
     *
     * @param offset
     *      Offset in buffer (in bytes) where the buffer starts.
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
    bool extractBuffer(std::size_t offset,
                       std::size_t wantedSize,
                       std::vector<char>& buffer) const;

private:
    /** Array with data. */
    const char* const m_data;

    /** Size of the data in bytes. */
    std::size_t m_size;

};

} // namespace protocol
} // namespace subttxrend

