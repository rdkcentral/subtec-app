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


#ifndef SUBTTXREND_TESTAPPS_DATASOURCEFACTORY_HPP_
#define SUBTTXREND_TESTAPPS_DATASOURCEFACTORY_HPP_

#include <string>
#include <vector>
#include <memory>

#include "DataSource.hpp"

namespace subttxrend
{
namespace testapps
{

/** Factory entry describing single source type. */
class DataSourceFactoryEntry
{
public:
    /**
     * Constructor.
     *
     * @param prefix
     *      Source type path prefix.
     * @param description
     *      Source type description.
     */
    DataSourceFactoryEntry(const std::string prefix,
                           const std::string description);

    /**
     * Destructor.
     */
    virtual ~DataSourceFactoryEntry() = default;

    /**
     * Constructor.
     *
     * @param path
     *      Source-specific path (without type prefix).
     *
     * @return
     *      Pointer to created source or null pointer on error.
     */
    virtual std::unique_ptr<DataSource> createSource(const std::string& path) const = 0;

    /**
     * Returns source type path prefix.
     *
     * @return
     *      Path prefix.
     */
    const std::string& getPrefix() const;

    /**
     * Returns source type description.
     *
     * @return
     *      Description.
     */
    const std::string& getDescription() const;

private:
    /** Path prefix. */
    const std::string m_prefix;

    /** Description. */
    const std::string m_description;
};

/**
 * Data source factory.
 */
class DataSourceFactory
{
public:
    /**
     * Constructor.
     */
    DataSourceFactory();

    /**
     * Destructor.
     */
    ~DataSourceFactory();

    /**
     * Creates the source.
     *
     * @param path
     *      Source path (including type prefix).
     *
     * @return
     *      Pointer to created source or null pointer on error.
     */
    std::unique_ptr<DataSource> createSource(const std::string& path) const;

    /**
     * Returns number of available source types.
     *
     * @return
     *      Number of types.
     */
    std::size_t getTypeCount() const;

    /**
     * Returns entry describing single avaialble source type.
     *
     * @param index
     *      Index of entry to get.
     *      Shall be smaller than getTypeCount().
     *
     * @return
     *      Entry describing the type.
     */
    const DataSourceFactoryEntry& getTypeInfo(std::size_t index) const;

private:

    /** Entries about available source types. */
    std::vector<std::unique_ptr<DataSourceFactoryEntry>> m_entries;
};

} // namespace testapps
} // namespace subttxrend

#endif /*SUBTTXREND_TESTAPPS_DATASOURCEFACTORY_HPP_*/
