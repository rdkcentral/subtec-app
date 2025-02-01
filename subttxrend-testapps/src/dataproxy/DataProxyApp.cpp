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


#include "DataProxyApp.hpp"

#include <cstdlib>
#include <iostream>

namespace subttxrend
{
namespace testapps
{

DataProxyApp::DataProxyApp(const std::string& appName,
                           const std::vector<std::string>& arguments) :
        m_appName(appName),
        m_arguments(arguments),
        m_sourceFactory(),
        m_targetFactory()
{
    // noop
}

DataProxyApp::~DataProxyApp()
{
    // noop
}

int DataProxyApp::run()
{
    // check arguments

    if (m_arguments.size() != 2)
    {
        printUsage();
        return EXIT_FAILURE;
    }

    const std::string sourcePath = m_arguments[0];
    const std::string targetPath = m_arguments[1];

    if (sourcePath.empty() || targetPath.empty())
    {
        printUsage();
        return EXIT_FAILURE;
    }

    std::unique_ptr<DataSource> dataSource = m_sourceFactory.createSource(
            sourcePath);
    std::unique_ptr<DataTarget> dataTarget = m_targetFactory.createTarget(
            targetPath);

    if (!dataSource || !dataSource->open())
    {
        std::cerr << "Cannot open source: " << sourcePath << std::endl;
        return EXIT_FAILURE;
    }

    if (!dataTarget || !dataTarget->open())
    {
        std::cerr << "Cannot open target: " << targetPath << std::endl;
        return EXIT_FAILURE;
    }

    std::unique_ptr<DataPacket> packet(new DataPacket(512 * 1024));

    int result = EXIT_SUCCESS;

    std::cout << "Packets processing loop started" << std::endl;

    while (dataTarget->wantsMorePackets())
    {
        if (!dataSource->readPacket(*packet))
        {
            std::cerr << "Packet read failed" << std::endl;
            result = EXIT_FAILURE;
            break;
        }

        if (packet->getSize() == 0)
        {
            break;
        }

        if (!dataTarget->writePacket(*packet))
        {
            std::cerr << "Packet write failed" << std::endl;
            result = EXIT_FAILURE;
            break;
        }
    }

    std::cout << "Packets processing loop finished" << std::endl;

    return result;
}

void DataProxyApp::printUsage() const
{
    std::cout << "Usage:" << std::endl;
    std::cout << m_appName << " <source-path> <target-path>" << std::endl;
    std::cout << std::endl;

    std::cout << "The <source-path> can be one of:" << std::endl;
    for (std::size_t i = 0; i < m_sourceFactory.getTypeCount(); ++i)
    {
        const auto& entry = m_sourceFactory.getTypeInfo(i);

        std::cout << "    " << entry.getDescription() << std::endl;
    }
    std::cout << std::endl;

    std::cout << "The <target-path> can be one of:" << std::endl;
    for (std::size_t i = 0; i < m_targetFactory.getTypeCount(); ++i)
    {
        const auto& entry = m_targetFactory.getTypeInfo(i);

        std::cout << "    " << entry.getDescription() << std::endl;
    }
    std::cout << std::endl;
}

} // namespace testapps
} // namespace subttxrend
