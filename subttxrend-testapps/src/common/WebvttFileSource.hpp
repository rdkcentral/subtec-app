/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/
#pragma once

#include "PlainFileSource.hpp"

namespace subttxrend
{
namespace testapps
{

/**
 * Ttml data file source. Ttml selection packet and contents of given file as ttml data content.
 */
class WebvttFileSource : public PlainFileSource
{
public:

    /**
     * Constructor.
     *
     * @param path
     *      Target path.
     */
    WebvttFileSource(const std::string& path);

    /**
     * Destructor.
     */
    virtual ~WebvttFileSource();

    /** @copydoc DataSource::readPacket() */
    virtual bool readPacket(DataPacket& packet) override;

private:
    /** RESET ALL. */
    std::uint32_t PACKET_TYPE_RESET_ALL = 3;

    /** Ttml selection packet id. */
    std::uint32_t PACKET_TYPE_WVTT_SELECTION = 16;

    /** Ttml data packet type. */
    std::uint32_t PACKET_TYPE_WVTT_DATA = 17;

    /** Ttml data packet type. */
    std::uint32_t PACKET_TYPE_WVTT_TIMESTAMP = 18;

};

} // namespace subttxrend
} // namespace testapps

