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

#include <stdexcept>
#include "DataTargetFactory.hpp"

#include "Ipv4SocketTarget.hpp"
#include "UnixSocketTarget.hpp"
#include "PlainFileTarget.hpp"
#include "SmartFileTarget.hpp"
#include "ConsoleLogTarget.hpp"

namespace subttxrend
{
namespace testapps
{

/**
 * Typed entry for creating specific target types.
 */
template<class TargetType>
class TypedFactoryEntry : public DataTargetFactoryEntry
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
    TypedFactoryEntry(const std::string prefix,
                      const std::string description);

    /** Destructor. */
    virtual ~TypedFactoryEntry() = default;

    /** @copydoc DataTargetFactoryEntry::createTarget */
    virtual std::unique_ptr<DataTarget> createTarget(const std::string& path) const
            override;
};

DataTargetFactoryEntry::DataTargetFactoryEntry(const std::string prefix,
                                               const std::string description) :
        m_prefix(prefix),
        m_description(description)
{
    // noop
}

const std::string& DataTargetFactoryEntry::getPrefix() const
{
    return m_prefix;
}

const std::string& DataTargetFactoryEntry::getDescription() const
{
    return m_description;
}

template<class TargetType>
TypedFactoryEntry<TargetType>::TypedFactoryEntry(const std::string prefix,
                                                 const std::string description) :
        DataTargetFactoryEntry(prefix, description)
{
    // noop
}

template<class TargetType>
std::unique_ptr<DataTarget> TypedFactoryEntry<TargetType>::createTarget(const std::string& path) const
{
    return std::unique_ptr<DataTarget>(new TargetType(path));
}

DataTargetFactory::DataTargetFactory()
{
    typedef std::unique_ptr<DataTargetFactoryEntry> EntryPtr;

    m_entries.push_back(EntryPtr(new TypedFactoryEntry<Ipv4SocketTarget>("ipv4",    "ipv4:<address> - IPv4 socket")));
    m_entries.push_back(EntryPtr(new TypedFactoryEntry<UnixSocketTarget>("unix",    "unix:<path>    - Unix socket")));
    m_entries.push_back(EntryPtr(new TypedFactoryEntry<PlainFileTarget> ("file",    "file:<path>    - Plain file")));
    m_entries.push_back(EntryPtr(new TypedFactoryEntry<SmartFileTarget> ("sfile",   "sfile:<path>   - Smart file (waits on timestamps)")));
    m_entries.push_back(EntryPtr(new TypedFactoryEntry<ConsoleLogTarget>("console", "console:       - Console (log only)")));

}

DataTargetFactory::~DataTargetFactory()
{
    // noop
}

std::unique_ptr<DataTarget> DataTargetFactory::createTarget(const std::string& path) const
{
    auto prefixSepIndex = path.find(':');
    if (prefixSepIndex != std::string::npos)
    {
        auto pathPrefix = path.substr(0, prefixSepIndex);
        auto pathData = path.substr(prefixSepIndex + 1);

        for (const auto& entry : m_entries)
        {
            if (entry->getPrefix() == pathPrefix)
            {
                return entry->createTarget(pathData);
            }
        }
    }

    return nullptr;
}

std::size_t DataTargetFactory::getTypeCount() const
{
    return m_entries.size();
}

const DataTargetFactoryEntry& DataTargetFactory::getTypeInfo(std::size_t index) const
{
    if (index < m_entries.size())
    {
        return *(m_entries[index].get());
    }
    else
    {
        throw std::length_error("Given index is out of range");
    }
}

} // namespace testapps
} // namespace subttxrend
