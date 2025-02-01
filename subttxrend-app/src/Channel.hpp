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


#ifndef _SUBTTXREND_APP_CHANNEL_HPP_
#define _SUBTTXREND_APP_CHANNEL_HPP_

#include <cassert>
#include <cstdint>

namespace subttxrend
{
namespace app
{

/**
 * Class represents channel. It may be active with given id or inactive one.
 */
class Channel
{
public:

    /**
     * Constructor.
     */
    Channel() :
            m_active(false),
            m_id()
    {
        // noop
    }

    /**
     * Destructor.
     */
    ~Channel() = default;

    /**
     * Sets current channel and make it active.
     *
     * @param channelId
     *      Channel identifier.
     */
    void set(std::uint32_t channelId)
    {
        m_id = channelId;
        m_active = true;
    }

    /**
     * Sets the channel to inactive.
     */
    void reset()
    {
        m_active = false;
        m_id = 0;
    }

    /**
     * Checks if channel is active.
     *
     * @return
     *      True if channel is active.
     */
    bool isActive() const
    {
        return m_active;
    }

    /**
     * Return current channel id.
     *
     * @return
     *      Channel id.
     */
    std::uint32_t getChannelId() const
    {
        // assume channel is active
        assert(m_active);
        return m_id;
    }

    /**
     * Checks if channel is active and matches given id.
     *
     * @param channelId
     *      Channel identifier.
     *
     * @return
     *      True if channel is active.
     */
    bool isEqual(std::uint32_t channelId) const
    {
        return (isActive() && (getChannelId() == channelId));
    }

private:
    /** Is channel active. */
    bool m_active;

    /** Selected channel. */
    std::uint32_t m_id;
};

} // namespace app
} // namespace subttxrend

#endif /* _SUBTTXREND_APP_CHANNEL_HPP_ */
