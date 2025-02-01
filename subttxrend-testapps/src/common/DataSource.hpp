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


#ifndef SUBTTXREND_TESTAPPS_DATASOURCE_HPP_
#define SUBTTXREND_TESTAPPS_DATASOURCE_HPP_

#include <string>

#include "DataPacket.hpp"

namespace subttxrend
{
namespace testapps
{

/**
 * Data source interface.
 *
 * The classes implementing this interface must close themselves
 * upon destruction.
 */
class DataSource
{
public:
    /**
     * Constructor.
     *
     * @param path
     *      Source specific path. To be parsed on open.
     */
    explicit DataSource(const std::string& path);

    /**
     * Destructor.
     */
    virtual ~DataSource() = default;

    /**
     * Opens the source.
     *
     * @retval true
     *      Source was successfully opened (or was already open).
     * @retval false
     *      Source cannot be opened.
     */
    virtual bool open() = 0;

    /**
     * Closes the source.
     */
    virtual void close() = 0;

    /**
     * Reads next packet from the source.
     *
     * @param packet
     *      Pointer to packet to be filled with data.
     *
     * @retval true
     *      Packet was read.
     *      If the size of packet read is zero then there was no more data.
     * @retval false
     *      Error occurred during read operation.
     */
    virtual bool readPacket(DataPacket& packet) = 0;

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

#endif /*SUBTTXREND_TESTAPPS_DATASOURCE_HPP_*/
