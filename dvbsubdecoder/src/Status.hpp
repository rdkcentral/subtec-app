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


#ifndef DVBSUBDECODER_STATUS_HPP_
#define DVBSUBDECODER_STATUS_HPP_

#include <cstdint>

#include "Types.hpp"

namespace dvbsubdecoder
{

/**
 * Decoder status information.
 *
 * Holds configuration, validity flag and other information
 * related to decoder status.
 */
class Status
{
public:
    /**
     * Constructor.
     *
     * @param specVersion
     *      Selected spec version.
     */
    Status(Specification specVersion);

    /**
     * Destructor.
     */
    ~Status() = default;

    /**
     * Returns selected specification version.
     *
     * @return
     *      Specification version.
     */
    Specification getSpecVersion() const
    {
        return m_specVersion;
    }

    /**
     * Sets ancillary/composition page ids to decode.
     *
     * @param compositionPageId
     *      Composition page id.
     * @param ancillaryPageId
     *      Ancillary page id.
     *      If ancillary page should not be used (e.g. because it is not
     *      present in the stream) the value shall be equal to composition
     *      page identifier.
     */
    void setPageIds(std::uint16_t compositionPageId,
                    std::uint16_t ancillaryPageId);

    /**
     * Checks if given page is selected.
     *
     * @param pageId
     *      Page id.
     *
     * @return
     *      True if given page id matches selected composition page id
     *      or ancillary page id, false otherwise.
     */
    bool isSelectedPage(std::uint16_t pageId) const;

    /**
     * Checks if given page is composition page id.
     *
     * @param pageId
     *      Page id.
     *
     * @return
     *      True if given page id matches selected composition page id,
     *      false otherwise.
     */
    bool isCompositionPage(std::uint16_t pageId) const;

    /**
     * Checks if given page is ancillary page id.
     *
     * @param pageId
     *      Page id.
     *
     * @return
     *      True if given page id matches selected ancillary page id,
     *      false otherwise.
     */
    bool isAncillaryPage(std::uint16_t pageId) const;

    /**
     * Sets last packet PTS.
     *
     * @param pts
     *      PTS value.
     */
    void setLastPts(const StcTime& pts)
    {
        m_lastPts = pts;
    }

    /**
     * Returns last packet PTS.
     *
     * @return
     *      Last packet PTS.
     */
    const StcTime& getLastPts() const
    {
        return m_lastPts;
    }

private:
    /** Version of specification to use. */
    const Specification m_specVersion;

    /** Page ids set flag. */
    bool m_pageIdsSet;

    /** Composition page id. */
    std::uint16_t m_compositionPageId;

    /** Ancillary page id. */
    std::uint16_t m_ancillaryPageId;

    /** Last processed packet PTS. */
    StcTime m_lastPts;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_STATUS_HPP_*/
