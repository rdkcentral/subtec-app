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


#ifndef WAYLANDCPP_KEYEVENT_HPP_
#define WAYLANDCPP_KEYEVENT_HPP_

#include <xkbcommon/xkbcommon.h>
#include <memory>

namespace waylandcpp
{

/**
 * Key event.
 */
struct KeyEvent
{
    /**
     * Event type.
     */
    enum class Type
    {
        /** Invalid. */
        INVALID,
        /** Key pressed. */
        PRESSED,
        /** Key released. */
        RELEASED
    };

    /** Modifier mask - alt. */
    static const uint32_t MODIFIER_MASK_ALT = 0x1;

    /** Modifier mask - shift. */
    static const uint32_t MODIFIER_MASK_SHIFT = 0x1;

    /** Modifier mask - control. */
    static const uint32_t MODIFIER_MASK_CONTROL = 0x1;

    /**
     * Constructor.
     */
    KeyEvent() :
            m_type(Type::INVALID),
            m_code(0),
            m_symbol(XKB_KEY_NoSymbol),
            m_modifiers()
    {
        // noop
    }

    /** Event type. */
    Type m_type;

    /** Key code. */
    xkb_keycode_t m_code;

    /** Event symbol. */
    xkb_keysym_t m_symbol;

    /** Modifiers. */
    uint32_t m_modifiers;
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_KEYEVENT_HPP_*/
