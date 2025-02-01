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


#ifndef _SUBTTXREND_DBUS_TELETEXTSTATUS_HPP_
#define _SUBTTXREND_DBUS_TELETEXTSTATUS_HPP_

namespace subttxrend
{
namespace dbus
{

/**
 * Simple class for holding teletext status response data.
 */
class TeletextStatus final
{
public:

    /**
     * Constructor.
     */
    TeletextStatus()
    {
        reset();
        m_muted = false;
    }

    /**
     * Constructor.
     *
     * @param started
     *      Teletext "started" state.
     *
     * @param muted
     *      Teletext "muted" state.
     */
    TeletextStatus(bool started,
                   bool muted,
                   int initialMagazine,
                   int initialPage) :
            m_started(started),
            m_muted(muted),
            m_initialMagazine(initialMagazine),
            m_initialPage(initialPage)
    {
        // noop
    }

    /**
     * Destructor.
     */
    ~TeletextStatus() = default;

    /**
     * Resets to default state.
     */
    void reset()
    {
        m_started = false;

        // preserve muted state

        m_initialMagazine = DEFAULT_INITIAL_MAGAZINE;
        m_initialPage = DEFAULT_INITIAL_PAGE;
    }

    /** Teletext "started" state. */
    bool m_started;

    /** Teletext "muted" state. */
    bool m_muted;

    /** Magazine number to start with. */
    int m_initialMagazine;

    /** Page_number to start with. */
    int m_initialPage;

    constexpr static int DEFAULT_INITIAL_MAGAZINE = 1;
    constexpr static int DEFAULT_INITIAL_PAGE = 0;
};

} // namespace dbus
} // namespace subttxrend

#endif /* _SUBTTXREND_DBUS_TELETEXTSTATUS_HPP_ */
