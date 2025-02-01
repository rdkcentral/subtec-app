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


#ifndef SUBTTXREND_GFX_KEY_EVENT_HPP_
#define SUBTTXREND_GFX_KEY_EVENT_HPP_

#include <xkbcommon/xkbcommon.h>

namespace subttxrend
{
namespace gfx
{

/**
 * Key event.
 */
class KeyEvent
{
public:
    /** Event type. */
    enum class Type
    {
        /** Key pressed. */
        PRESSED,
        /** Key released. */
        RELEASED
    };

    /**
     * Constructor.
     *
     * @param type
     *      Event type.
     * @param symbol
     *      Key symbol.
     */
    KeyEvent(Type type,
             xkb_keysym_t symbol) :
            m_type(type),
            m_symbol(symbol)
    {
        // noop
    }

    /**
     * Returns event type.
     *
     * @return
     *      Event type.
     */
    Type getType() const
    {
        return m_type;
    }

    /**
     * Returns key symbol.
     *
     * @return
     *      Key symbol.
     */
    xkb_keysym_t getSymbol() const
    {
        return m_symbol;
    }

private:
    /** Event type. */
    Type m_type;

    /** Key symbol. */
    xkb_keysym_t m_symbol;
};

} // namespace gfx
} // namespace subttxrend

#endif /*SUBTTXREND_GFX_KEY_EVENT_HPP_*/
