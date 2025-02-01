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


#ifndef SUBTTXREND_TESTAPPS_DATATARGET_HPP_
#define SUBTTXREND_TESTAPPS_DATATARGET_HPP_

#include <string>

#include "DataPacket.hpp"

namespace subttxrend
{
namespace testapps
{

/**
 * Data target interface.
 *
 * The classes implementing this interface must close themselves
 * upon destruction.
 */
class DataTarget
{
public:
    /**
     * Constructor.
     *
     * @param path
     *      Target specific path. To be parsed on open.
     */
    explicit DataTarget(const std::string& path);

    /**
     * Destructor.
     */
    virtual ~DataTarget() = default;

    /**
     * Opens the target.
     *
     * @retval true
     *      Target was successfully opened (or was already open).
     * @retval false
     *      Target cannot be opened.
     */
    virtual bool open() = 0;

    /**
     * Closes the target.
     */
    virtual void close() = 0;

    /**
     * Checks if targets wants more packets.
     *
     * @retval true
     *      Target wants next packet.
     * @retval false
     *      Target does not want more packets.
     */
    virtual bool wantsMorePackets() = 0;

    /**
     * Writes packet.
     *
     * @param packet
     *      Pointer to packet to be written.
     *
     * @retval true
     *      Packet was successsfully written.
     * @retval false
     *      Error occurred during write operation.
     */
    virtual bool writePacket(const DataPacket& packet) = 0;

    /**
     * Returns path.
     *
     * @return
     *      Path given to constructor.
     */
    const std::string& getPath() const;

private:
    /** Path. */
    const std::string m_path;
};

} // namespace testapps
} // namespace subttxrend

#endif /*SUBTTXREND_TESTAPPS_DATATARGET_HPP_*/
