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


#include "DataSourceFactory.hpp"

#include "Ipv4SocketSource.hpp"
#include "UnixSocketSource.hpp"
#include "PlainFileSource.hpp"
#include "SmartFileSource.hpp"
#include "RandomPacketSource.hpp"
#include "TtmlFileSource.hpp"
#include "WebvttFileSource.hpp"
#include "WebvttFileNoHeaderSource.hpp"

namespace subttxrend
{
namespace testapps
{

/**
 * Typed entry for creating specific source types.
 */
template<class SourceType>
class TypedFactoryEntry : public DataSourceFactoryEntry
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
    TypedFactoryEntry(const std::string prefix,
                      const std::string description);

    /** Destructor. */
    virtual ~TypedFactoryEntry() = default;

    /** @copydoc DataSourceFactoryEntry::createSource */
    virtual std::unique_ptr<DataSource> createSource(const std::string& path) const
            override;
};

DataSourceFactoryEntry::DataSourceFactoryEntry(const std::string prefix,
                                               const std::string description) :
        m_prefix(prefix),
        m_description(description)
{
    // noop
}

const std::string& DataSourceFactoryEntry::getPrefix() const
{
    return m_prefix;
}

const std::string& DataSourceFactoryEntry::getDescription() const
{
    return m_description;
}

template<class SourceType>
TypedFactoryEntry<SourceType>::TypedFactoryEntry(const std::string prefix,
                                                 const std::string description) :
        DataSourceFactoryEntry(prefix, description)
{
    // noop
}

template<class SourceType>
std::unique_ptr<DataSource> TypedFactoryEntry<SourceType>::createSource(const std::string& path) const
{
    return std::unique_ptr<DataSource>(new SourceType(path));
}

DataSourceFactory::DataSourceFactory()
{
    typedef std::unique_ptr<DataSourceFactoryEntry> EntryPtr;

    m_entries.push_back(EntryPtr(new TypedFactoryEntry<Ipv4SocketSource>   ("ipv4",  "ipv4:<address> - IPv4 socket")));
    m_entries.push_back(EntryPtr(new TypedFactoryEntry<UnixSocketSource>   ("unix",  "unix:<path>    - Unix socket")));
    m_entries.push_back(EntryPtr(new TypedFactoryEntry<PlainFileSource>    ("file",  "file:<path>    - Plain file")));
    m_entries.push_back(EntryPtr(new TypedFactoryEntry<TtmlFileSource>     ("ttml",  "ttml:<path>    - Ttml content file")));
    m_entries.push_back(EntryPtr(new TypedFactoryEntry<WebvttFileSource>     ("wvtt",  "wvtt:<path>    - WebVTT content file")));
    m_entries.push_back(EntryPtr(new TypedFactoryEntry<WebvttFileNoHeaderSource>     ("wvttnh",  "wvtt:<path>    - WebVTT content file")));
    // m_entries.push_back(EntryPtr(new TypedFactoryEntry<ResetSource>     ("reset",  "wvtt:ts type (wvtt|ttml)")));
    m_entries.push_back(EntryPtr(new TypedFactoryEntry<SmartFileSource>    ("sfile", "sfile:<path>   - Smart file (waits on timestamps)")));
    m_entries.push_back(EntryPtr(new TypedFactoryEntry<RandomPacketSource> ("rand",  "rand:[<packet-count>:<sleep-ms>] - Random packets.")));

}

DataSourceFactory::~DataSourceFactory()
{
    // noop
}

std::unique_ptr<DataSource> DataSourceFactory::createSource(const std::string& path) const
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
                return entry->createSource(pathData);
            }
        }
    }

    return nullptr;
}

std::size_t DataSourceFactory::getTypeCount() const
{
    return m_entries.size();
}

const DataSourceFactoryEntry& DataSourceFactory::getTypeInfo(std::size_t index) const
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
