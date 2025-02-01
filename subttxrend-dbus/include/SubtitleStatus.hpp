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


#ifndef _SUBTTXREND_DBUS_SUBTITLESTATUS_HPP_
#define _SUBTTXREND_DBUS_SUBTITLESTATUS_HPP_

namespace subttxrend
{
namespace dbus
{

/**
 * Simple class for holding subtitles status response data.
 */
class SubtitleStatus final
{
public:

    /** Possible subtitle types. */
    enum class SubtitleType
    {
        DVB = 0, TTX = 1, SCTE_27, OTHER
    };

    /**
     * Constructor.
     */
    SubtitleStatus()
    {
        reset();
        m_muted = false;
    }

    /**
     * Constructor.
     *
     * @param started
     *      Subtitle "started" state.
     *
     * @param muted
     *      Subtitle "muted" state.
     *
     * @param subtitleType
     *      Subtitle type.
     *
     * @param auxId1
     *      Subtitle composition_page_id / magazine_nbr.
     *
     * @param auxId2
     *      Subtitle ancillary_page_id / page_number.
     */
    SubtitleStatus(bool started,
                   bool muted,
                   SubtitleType subtitleType,
                   int auxId1,
                   int auxId2) :
            m_started(started),
            m_muted(muted),
            m_type(subtitleType),
            m_auxId1(auxId1),
            m_auxId2(auxId2)
    {
        // noop
    }

    /**
     * Resets to default state.
     */
    void reset()
    {
        m_started = false;
        m_type = SubtitleType::OTHER;
        m_auxId1 = 0;
        m_auxId2 = 0;

        // preserve muted state
    }

    /**
     * Destructor.
     */
    ~SubtitleStatus() = default;

    /** Subtitle "started" state. */
    bool m_started;

    /** Subtitle "muted" state. */
    bool m_muted;

    /** Subtitle type. */
    SubtitleType m_type;

    /** Subtitle composition_page_id / magazine_nbr. */
    int m_auxId1;

    /** Subtitle ancillary_page_id / page_number. */
    int m_auxId2;
};

} // namespace dbus
} // namespace subttxrend

#endif /* _SUBTTXREND_DBUS_SUBTITLESTATUS_HPP_ */
