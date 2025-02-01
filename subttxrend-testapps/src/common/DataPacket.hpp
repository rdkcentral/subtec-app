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


#ifndef SUBTTXREND_TESTAPPS_DATAPACKET_HPP_
#define SUBTTXREND_TESTAPPS_DATAPACKET_HPP_

#include <memory>
#include <cstdint>

namespace subttxrend
{
namespace testapps
{

/**
 * Data file abstraction.
 */
class DataPacket
{
public:
    /**
     * Constructor.
     *
     * Creates data packet with specified capacity.
     *
     * @param capacity
     *      Maximum capacity in bytes.
     */
    DataPacket(const std::size_t capacity);

    /**
     * Destructor.
     */
    ~DataPacket() = default;

    /**
     * Returns packet buffer.
     *
     * @return
     *      Pointer to buffer.
     */
    char* getBuffer();

    /**
     * Returns packet buffer.
     *
     * @return
     *      Pointer to buffer.
     */
    const char* getBuffer() const;

    /**
     * Returns maximum capacity.
     *
     * @return
     *      Maximum capacity in bytes.
     */
    size_t getCapacity() const;

    /**
     * Returns current size.
     *
     * @return
     *      Number of bytes currently filled with data.
     */
    size_t getSize() const;

    /**
     * Sets size.
     *
     * @param newSize
     *      Number of bytes currently filled with data.
     *      Must not be greater than the capacity.
     */
    void setSize(std::size_t newSize);

    /**
     * Sets size to zero.
     */
    void reset();

    /**
     * Appends 32-bit unsigned integer in little endian notation.
     *
     * @param value
     *      Value to append.
     */
    void appendLeUint32(std::uint32_t value);
    void appendLeUint64(std::uint64_t value);

    /**
     * Appends requested number of zero bytes.
     */
    void appendZeroes(std::size_t count);


private:
    /** Capacity. */
    const std::size_t m_capacity;

    /** Current size. */
    std::size_t m_size;

    /** Buffer for data bytes. */
    std::unique_ptr<char> m_buffer;
};

} // namespace testapps
} // namespace subttxrend

#endif /*SUBTTXREND_TESTAPPS_DATAPACKET_HPP_*/
