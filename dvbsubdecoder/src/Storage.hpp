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


#ifndef DVBSUBDECODER_STORAGE_HPP_
#define DVBSUBDECODER_STORAGE_HPP_

#include "Database.hpp"
#include "PesBuffer.hpp"
#include "PixmapAllocator.hpp"

namespace dvbsubdecoder
{

class Allocator;

/**
 * Data storage.
 *
 * The storage class is a central point for storing all the data including
 * current state, decoded subtitles data and buffered PES packets.
 *
 * The class shall be a hub for the data and delegate storing of specific
 * data to its members.
 */
class Storage
{
public:
    /**
     * Constructor.
     *
     * @param specVersion
     *      Specification version.
     * @param allocator
     *      User memory allocator.
     * @param client
     *      Decoder client interface.
     */
    Storage(Specification specVersion,
            Allocator& allocator,
            DecoderClient& client);

    /**
     * Returns database.
     *
     * @return
     *      Database object.
     */
    Database& getDatabase();

    /**
     * Returns PES buffer.
     *
     * @return
     *      PES buffer.
     */
    PesBuffer& getPesBuffer();

private:
    /** Buffer for PES packets. */
    PesBuffer m_pesBuffer;

    /** Allocator for pixmap memory. */
    PixmapAllocator m_pixmapAllocator;

    /** Parsed subtitles elements. */
    Database m_database;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_STORAGE_HPP_*/
