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


#include "KeymapXkbV1.hpp"

#include <sys/mman.h>
#include <wayland-client.h>
#include <cstdlib>

#define SUPPORT_XKBCOMMON_COMPOSE

#include <xkbcommon/xkbcommon.h>
#ifdef SUPPORT_XKBCOMMON_COMPOSE
#include <xkbcommon/xkbcommon-compose.h>
#endif

#include <subttxrend/common/Logger.hpp>

namespace
{

subttxrend::common::Logger g_logger("GfxEngine", "KeymapXkbV1");

size_t simple_strnlen(const char* s, size_t max_len)
{
    size_t i = 0;
    for(; (i < max_len) && s[i]; ++i);
    return i;
}

} // namespace <anonymous>

namespace waylandcpp
{

KeymapXkbV1::KeymapXkbV1() :
        m_xkbContext(),
        m_xkbKeymap(),
        m_xkbState(),
        m_xkbMaskControl(0),
        m_xkbMaskAlt(0),
        m_xkbMaskShift(0),
        m_xkbComposeTable(),
        m_xkbComposeState(),
        m_modifiers(0)
{
    // noop
}

KeymapXkbV1::~KeymapXkbV1()
{
#ifdef SUPPORT_XKBCOMMON_COMPOSE
    if (m_xkbComposeState)
    {
        ::xkb_compose_state_unref(m_xkbComposeState);
    }

    if (m_xkbComposeTable)
    {
        ::xkb_compose_table_unref(m_xkbComposeTable);
    }
#endif

    if (m_xkbKeymap)
    {
        ::xkb_keymap_unref(m_xkbKeymap);
    }

    if (m_xkbState)
    {
        ::xkb_state_unref(m_xkbState);
    }

    if (m_xkbContext)
    {
        ::xkb_context_unref(m_xkbContext);
    }
}

bool KeymapXkbV1::init(int32_t fd,
                       uint32_t size)
{
    m_xkbContext = ::xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!m_xkbContext)
    {
        g_logger.warning("%s - Failed to create XKB context", __func__);
        return false;
    }

    void* mapPtr = ::mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapPtr == MAP_FAILED)
    {
        return false;
    }

    const char* mapStr = reinterpret_cast<char*>(mapPtr);

    /* Set up XKB keymap */
    m_xkbKeymap = ::xkb_keymap_new_from_buffer(m_xkbContext, mapStr,
            simple_strnlen(mapStr, size), XKB_KEYMAP_FORMAT_TEXT_V1,
            XKB_KEYMAP_COMPILE_NO_FLAGS);
    ::munmap(mapPtr, size);

    if (!m_xkbKeymap)
    {
        g_logger.warning("%s - failed to compile keymap", __func__);

        m_xkbKeymap = xkb_keymap_new_from_names(m_xkbContext, NULL,
                XKB_KEYMAP_COMPILE_NO_FLAGS);
        if (!m_xkbKeymap)
        {
            g_logger.warning("%s - failed to compile default keymap",
                    __func__);

            return false;
        }
    }

    /* Set up XKB state */
    m_xkbState = ::xkb_state_new(m_xkbKeymap);
    if (!m_xkbState)
    {
        g_logger.warning("%s - failed to create XKB state", __func__);
        return false;
    }

    /* Set up XKB compose table */
#ifdef SUPPORT_XKBCOMMON_COMPOSE
    /* Look up the preferred locale, falling back to "C" as default */
    const char* locale = nullptr;
    if (!(locale = ::getenv("LC_ALL")))
    {
        if (!(locale = ::getenv("LC_CTYPE")))
        {
            if (!(locale = ::getenv("LANG")))
            {
                locale = "C";
            }
        }
    }

    m_xkbComposeTable = ::xkb_compose_table_new_from_locale(m_xkbContext,
            locale, XKB_COMPOSE_COMPILE_NO_FLAGS);
    if (m_xkbComposeTable)
    {
        /* Set up XKB compose state */
        m_xkbComposeState = ::xkb_compose_state_new(m_xkbComposeTable,
                XKB_COMPOSE_STATE_NO_FLAGS);
        if (!m_xkbComposeState)
        {
            g_logger.warning(
                    "%s - could not create XKB compose state. Disabiling compose.",
                    __func__);
        }
    }
    else
    {
        g_logger.warning(
                "%s - could not create XKB compose table for locale '%s'.  "
                        "Disabiling compose", __func__, locale);
    }
#endif

    m_xkbMaskControl = 1 << ::xkb_keymap_mod_get_index(m_xkbKeymap, "Control");
    m_xkbMaskAlt = 1 << ::xkb_keymap_mod_get_index(m_xkbKeymap, "Mod1");
    m_xkbMaskShift = 1 << ::xkb_keymap_mod_get_index(m_xkbKeymap, "Shift");

    return true;
}

KeyEvent KeymapXkbV1::processKey(uint32_t key,
                                 uint32_t state)
{
    KeyEvent event;

    event.m_type = KeyEvent::Type::INVALID;

    if (state == WL_KEYBOARD_KEY_STATE_RELEASED)
    {
        event.m_type = KeyEvent::Type::RELEASED;
    }
    else if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
    {
        event.m_type = KeyEvent::Type::PRESSED;
    }

    event.m_code = key + 8;
    event.m_symbol = xkb_state_key_get_one_sym(m_xkbState, event.m_code);

    if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
    {
        event.m_symbol = processKeyPress(event.m_symbol);
    }

    return event;
}

void KeymapXkbV1::processModifiers(uint32_t mods_depressed,
                                   uint32_t mods_latched,
                                   uint32_t mods_locked,
                                   uint32_t group)
{
    /* If we're not using a keymap, then we don't handle PC-style modifiers */
    if (!m_xkbKeymap)
    {
        return;
    }

    ::xkb_state_update_mask(m_xkbState, mods_depressed, mods_latched,
            mods_locked, 0, 0, group);

    xkb_mod_mask_t mask = ::xkb_state_serialize_mods(m_xkbState,
            static_cast<xkb_state_component>(XKB_STATE_MODS_DEPRESSED
                    | XKB_STATE_MODS_LATCHED));

    m_modifiers = 0;
    m_modifiers |=
            ((mask & m_xkbMaskControl) != 0) ?
                    KeyEvent::MODIFIER_MASK_CONTROL : 0;
    m_modifiers |=
            ((mask & m_xkbMaskAlt) != 0) ? KeyEvent::MODIFIER_MASK_ALT : 0;
    m_modifiers |=
            ((mask & m_xkbMaskShift) != 0) ? KeyEvent::MODIFIER_MASK_SHIFT : 0;
}

xkb_keysym_t KeymapXkbV1::processKeyPress(xkb_keysym_t sym)
{
#ifdef SUPPORT_XKBCOMMON_COMPOSE
    if (!m_xkbComposeState)
    {
        return sym;
    }

    if (sym == XKB_KEY_NoSymbol)
    {
        return sym;
    }

    if (::xkb_compose_state_feed(m_xkbComposeState, sym)
            != XKB_COMPOSE_FEED_ACCEPTED)
        return sym;

    switch (::xkb_compose_state_get_status(m_xkbComposeState))
    {
    case XKB_COMPOSE_COMPOSING:
        return XKB_KEY_NoSymbol;
    case XKB_COMPOSE_COMPOSED:
        return ::xkb_compose_state_get_one_sym(m_xkbComposeState);
    case XKB_COMPOSE_CANCELLED:
        return XKB_KEY_NoSymbol;
    case XKB_COMPOSE_NOTHING:
        return sym;
    default:
        return sym;
    }
#else
    return sym;
#endif
}

} // namespace waylandcpp
