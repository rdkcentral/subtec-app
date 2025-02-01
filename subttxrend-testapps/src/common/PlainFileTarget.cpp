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


#include "PlainFileTarget.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

namespace subttxrend
{
namespace testapps
{

PlainFileTarget::PlainFileTarget(const std::string& path) :
        DataTarget(path),
        m_fileHandle(-1)
{
    // noop
}

PlainFileTarget::~PlainFileTarget()
{
    close();
}

bool PlainFileTarget::open()
{
    if (m_fileHandle != -1)
    {
        return true;
    }

    m_fileHandle = ::open(getPath().c_str(), O_CREAT | O_TRUNC | O_WRONLY,
            0664);
    if (m_fileHandle == -1)
    {
        std::cerr << "Cannot open file" << std::endl;
        return false;
    }

    return true;
}

void PlainFileTarget::close()
{
    if (m_fileHandle != -1)
    {
        (void) ::close(m_fileHandle);
        m_fileHandle = -1;
    }
}

bool PlainFileTarget::wantsMorePackets()
{
    return (m_fileHandle != -1);
}

bool PlainFileTarget::writePacket(const DataPacket& packet)
{
    if (m_fileHandle == -1)
    {
        return false;
    }

    auto size = packet.getSize();
    auto written = ::write(m_fileHandle, packet.getBuffer(), size);

    std::cout << "Write operation - requested: " << size << " written: "
            << written << std::endl;

    return written == static_cast<int>(size);
}

} // namespace testapps
} // namespace subttxrend
