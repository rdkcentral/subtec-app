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


#ifndef SUBTTXREND_TESTAPPS_DATAPROXYAPP_HPP_
#define SUBTTXREND_TESTAPPS_DATAPROXYAPP_HPP_

#include <string>
#include <memory>
#include <vector>

#include <common/DataSourceFactory.hpp>
#include <common/DataTargetFactory.hpp>

namespace subttxrend
{
namespace testapps
{

class DataSource;
class DataTarget;

/**
 * Main application class.
 */
class DataProxyApp
{
public:
    /**
     * Constructor.
     *
     * @param appName
     *      Name used to run the application.
     * @param arguments
     *      Array of arguments passed to application.
     */
    DataProxyApp(const std::string& appName,
                 const std::vector<std::string>& arguments);

    /**
     * Destructor.
     */
    ~DataProxyApp();

    /**
     * Runs the application.
     *
     * @return
     *      Application exit code.
     */
    int run();

private:
    /**
     * Prints command line usage information.
     */
    void printUsage() const;

    /** Name used to run the application. */
    const std::string m_appName;

    /** Array of arguments passed to application. */
    const std::vector<std::string> m_arguments;

    /** Factory for data sources. */
    DataSourceFactory m_sourceFactory;

    /** Factory for data targets. */
    DataTargetFactory m_targetFactory;
};

} // namespace testapps
} // namespace subttxrend

#endif /*SUBTTXREND_TESTAPPS_DATAPROXYAPP_HPP_*/
