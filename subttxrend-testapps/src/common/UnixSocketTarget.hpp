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


#ifndef SUBTTXREND_TESTAPPS_UNIXSOCKETTARGET_HPP_
#define SUBTTXREND_TESTAPPS_UNIXSOCKETTARGET_HPP_

#include <string>

#include "DataTarget.hpp"

namespace subttxrend
{
namespace testapps
{

/**
 * Unix socket based target.
 */
class UnixSocketTarget : public DataTarget
{
public:
    /**
     * Constructor.
     *
     * @param path
     *      Target path.
     */
    UnixSocketTarget(const std::string& path);

    /**
     * Destructor.
     */
    virtual ~UnixSocketTarget();

    /** @copydoc DataTarget::open() */
    virtual bool open() override;

    /** @copydoc DataTarget::close() */
    virtual void close() override;

    /** @copydoc DataTarget::wantsMorePackets() */
    virtual bool wantsMorePackets() override;

    /** @copydoc DataTarget::writePacket() */
    virtual bool writePacket(const DataPacket& packet) override;

private:
    /** Socket handle. */
    int m_socketHandle;
};

} // namespace testapps
} // namespace subttxrend

#endif /*SUBTTXREND_TESTAPPS_UNIXSOCKETTARGET_HPP_*/
