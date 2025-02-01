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


#ifndef SUBTTXREND_TESTAPPS_DATATARGETFACTORY_HPP_
#define SUBTTXREND_TESTAPPS_DATATARGETFACTORY_HPP_

#include <string>
#include <vector>
#include <memory>

#include "DataTarget.hpp"

namespace subttxrend
{
namespace testapps
{

/** Factory entry describing single target type. */
class DataTargetFactoryEntry
{
public:
    /**
     * Constructor.
     *
     * @param prefix
     *      Target type path prefix.
     * @param description
     *      Target type description.
     */
    DataTargetFactoryEntry(const std::string prefix,
                           const std::string description);

    /**
     * Destructor.
     */
    virtual ~DataTargetFactoryEntry() = default;

    /**
     * Constructor.
     *
     * @param path
     *      Target-specific path (without type prefix).
     *
     * @return
     *      Pointer to created target or null pointer on error.
     */
    virtual std::unique_ptr<DataTarget> createTarget(const std::string& path) const = 0;

    /**
     * Returns target type path prefix.
     *
     * @return
     *      Path prefix.
     */
    const std::string& getPrefix() const;

    /**
     * Returns target type description.
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
 * Data target factory.
 */
class DataTargetFactory
{
public:
    /**
     * Constructor.
     */
    DataTargetFactory();

    /**
     * Destructor.
     */
    ~DataTargetFactory();

    /**
     * Creates the target.
     *
     * @param path
     *      Target path (including type prefix).
     *
     * @return
     *      Pointer to created target or null pointer on error.
     */
    std::unique_ptr<DataTarget> createTarget(const std::string& path) const;

    /**
     * Returns number of available target types.
     *
     * @return
     *      Number of types.
     */
    std::size_t getTypeCount() const;

    /**
     * Returns entry describing single avaialble target type.
     *
     * @param index
     *      Index of entry to get.
     *      Shall be smaller than getTypeCount().
     *
     * @return
     *      Entry describing the type.
     */
    const DataTargetFactoryEntry& getTypeInfo(std::size_t index) const;

private:

    /** Entries about available target types. */
    std::vector<std::unique_ptr<DataTargetFactoryEntry>> m_entries;
};

} // namespace testapps
} // namespace subttxrend

#endif /*SUBTTXREND_TESTAPPS_DATATARGETFACTORY_HPP_*/
