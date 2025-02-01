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

#include "Source.hpp"
#include "PacketReceiver.hpp"
#include "UnixSocket.hpp"

#include <subttxrend/common/Logger.hpp>
#include <subttxrend/common/DataBuffer.hpp>
#include <subttxrend/protocol/PacketParser.hpp>
#include <subttxrend/protocol/StreamValidator.hpp>

#include <atomic>
#include <memory>
#include <thread>
#include <string>

namespace subttxrend
{
namespace socksrc
{

class PacketParser;

/**
 * Unix socket based data source.
 */
class UnixSocketSource : public Source
{
public:

    /**
     * Constructor.
     *
     * @param socketPath
     *      Unix socket path to use.
     */
    UnixSocketSource(std::string const& socketPath);

    /**
     * Destructor.
     */
    ~UnixSocketSource();

    /** @copydoc Source::start */
    virtual void start(PacketReceiver* receiver) override;

    /** @copydoc Source::stop */
    virtual void stop() override;

private:

    void createSocket();
    void sourceLoop();

    /**
     * Processes the received packet.
     *
     * @param buffer
     *      Pointer to buffer with packet data.
     */
    void processPacket(common::DataBufferPtr buffer);

    /** Unix socket path to use. */
    const std::string m_socketPath;

    /** Packets parser. */
    protocol::PacketParser m_parser;

    /** Packet stream validator. */
    protocol::StreamValidator m_validator;

    /** Object to be called when packet is received. */
    PacketReceiver* m_receiver{};

    UnixSocketPtr m_socket;
    std::thread m_recvThread;
    std::atomic_bool m_sourceRunning{false};

    /** Logger to use. */
    static common::Logger m_logger;
};

} // namespace socksrc
} // namespace subttxrend

