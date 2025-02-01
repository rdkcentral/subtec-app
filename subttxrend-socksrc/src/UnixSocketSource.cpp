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


#include "UnixSocketSource.hpp"
#include "UnixSocket.hpp"

#include <subttxrend/protocol/Packet.hpp>

#include <cstring>

namespace subttxrend
{
namespace socksrc
{

common::Logger UnixSocketSource::m_logger("SockSrc", "UnixSocketSource");

UnixSocketSource::UnixSocketSource(std::string const& socketPath) :
        m_socketPath(socketPath)
{
    // noop
}

UnixSocketSource::~UnixSocketSource()
{
    if (m_socket) {
        m_logger.oswarning(__LOGGER_FUNC__, " Source not stopped before destruction");
        stop();
    }
}

void UnixSocketSource::start(PacketReceiver* receiver)
{
    m_logger.osinfo(__LOGGER_FUNC__);

    assert(receiver);
    m_receiver = receiver;

    m_logger.osinfo(__LOGGER_FUNC__, " starting receiver thread");

    m_sourceRunning.store(true, std::memory_order_relaxed);
    m_recvThread = std::thread(&UnixSocketSource::sourceLoop, this);

    m_logger.osinfo(__LOGGER_FUNC__, " started");

}

void UnixSocketSource::stop()
{
    m_logger.osinfo(__LOGGER_FUNC__, " stopping...");

    m_sourceRunning.store(false, std::memory_order_relaxed);

    assert(m_socket);
    m_socket->shutdown();

    m_logger.osinfo(__LOGGER_FUNC__, " socket stopped, waiting for recv thread to finish...");

    if (m_recvThread.joinable()) {
        m_recvThread.join();
    }

    m_logger.osinfo(__LOGGER_FUNC__, " thread joined, source stopped");

    m_receiver = nullptr;
    m_socket.reset();
}

void UnixSocketSource::createSocket()
{
    m_logger.osinfo(__LOGGER_FUNC__);

    while ((! m_socket) && m_sourceRunning.load(std::memory_order_relaxed)) {
        try {
            m_socket = std::make_unique<UnixSocket>(m_socketPath);
            m_logger.osinfo(__LOGGER_FUNC__, " socket created, buffer size=", m_socket->getSocketBufferSize());
        }
        catch (UnixSocket::SocketException const& e) {
            m_logger.oserror(__LOGGER_FUNC__, " exception while creating socket: ", e.what());

            using namespace std::chrono_literals;
            auto constexpr SOCKET_RECREATE_SLEEP = 50ms;
            std::this_thread::sleep_for(SOCKET_RECREATE_SLEEP);
        }
    }
}

void UnixSocketSource::sourceLoop()
{
    m_logger.osinfo(__LOGGER_FUNC__, " start");

    using protocol::Packet;

    auto const headerSize = Packet::getHeaderSize();
    common::DataBuffer headerBuffer(headerSize);

    createSocket();
    while(m_sourceRunning.load(std::memory_order_relaxed)) {

        auto socketRestartNeeded = false;

        try {
            auto const bytesPeeked = m_socket->peekBytes(headerBuffer);
            if (bytesPeeked == headerSize) {
                auto const packetSize = headerSize + Packet::getSizeFromHeader(headerBuffer);
                auto buffer = std::make_unique<common::DataBuffer>(packetSize);
                m_socket->readBytes(*buffer);
                processPacket(std::move(buffer));
            }
            else if (bytesPeeked != 0) {
                m_logger.oserror(__LOGGER_FUNC__, " not enough header bytes read: ", bytesPeeked, " of ", headerSize);
                socketRestartNeeded = true;
            }
        }
        catch(UnixSocket::SocketException const& e)
        {
            m_logger.oserror(__LOGGER_FUNC__, " error reading from socket", e.what());
            socketRestartNeeded = true;
        }

        if (socketRestartNeeded) {
            m_logger.oserror(__LOGGER_FUNC__, " restarting socket");
            m_socket.reset();
            createSocket();
        }
    }
}

void UnixSocketSource::processPacket(common::DataBufferPtr dataBuffer)
{
    m_logger.ostrace(__LOGGER_FUNC__, " - Processing packet. Size: ", dataBuffer->size());

    bool streamWasValid = m_validator.isValid();
    bool streamIsValid = true;

    assert(m_receiver);

    using protocol::Packet;
    if (Packet::isDataPacket(Packet::getType(*dataBuffer)))
    {
        if (m_validator.validateCounter(Packet::getCounter(*dataBuffer)))
        {
            m_receiver->addBuffer(std::move(dataBuffer));
        }
        else
        {
            m_logger.oswarning(__LOGGER_FUNC__, " invalid counter in data packet");
            streamIsValid = false;
        }
    }
    else
    {
        const Packet& packet = m_parser.parse(std::move(dataBuffer));

        if (m_validator.validate(packet))
        {
            m_logger.osdebug(__LOGGER_FUNC__, " - Packet parsed: ", static_cast<unsigned>(packet.getType()));
            m_receiver->onPacketReceived(packet);
        } else {
            m_logger.oserror(__LOGGER_FUNC__, " - Packet parsing failed");
            streamIsValid = false;
        }
    }

    if (streamWasValid && ! streamIsValid)
    {
        m_logger.oserror(__LOGGER_FUNC__, " - Forcing RESET ALL due to broken stream");
        m_receiver->onStreamBroken();
    }

}

} // namespace socksrc
} // namespace subttxrend
