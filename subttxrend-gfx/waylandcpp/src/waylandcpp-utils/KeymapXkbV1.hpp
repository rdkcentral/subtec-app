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


#ifndef WAYLANDCPP_KEYMAPXKBV1_HPP_
#define WAYLANDCPP_KEYMAPXKBV1_HPP_

#include "Keymap.hpp"

// forwards
struct xkb_compose_table;
struct xkb_compose_state;

namespace waylandcpp
{

/**
 * Keymap - XKB V1.
 */
class KeymapXkbV1 : public Keymap
{
public:
    /**
     * Constructor.
     */
    KeymapXkbV1();

    /**
     * Destructor.
     */
    virtual ~KeymapXkbV1();

    /**
     * Initialize.
     *
     * @param fd
     *      keymap file descriptor
     * @param size
     *      keymap size, in bytes
     *
     * @retval true
     *      Success.
     * @retval false
     *      Failure.
     */
    bool init(int32_t fd,
              uint32_t size);

    /** @copydoc Keymap::processKey */
    virtual KeyEvent processKey(uint32_t key,
                                uint32_t state) override;

    /** @copydoc Keymap::processModifiers */
    virtual void processModifiers(uint32_t mods_depressed,
                                  uint32_t mods_latched,
                                  uint32_t mods_locked,
                                  uint32_t group) override;

private:
    /**
     * Translate symbols appropriately if a compose sequence is being entered.
     *
     * @param sym
     *      Received symbol.
     *
     * @return
     *       Translated symbol.
     */
    xkb_keysym_t processKeyPress(xkb_keysym_t sym);

    /** Library context. */
    xkb_context* m_xkbContext;

    /** Compiled keymap. */
    xkb_keymap* m_xkbKeymap;

    /** Keyboard state. */
    xkb_state* m_xkbState;

    /** Control key modifier mask. */
    xkb_mod_index_t m_xkbMaskControl;

    /** Alt key modifier mask. */
    xkb_mod_index_t m_xkbMaskAlt;

    /** Shift key modifier mask. */
    xkb_mod_index_t m_xkbMaskShift;

    /** Character compose table. */
    xkb_compose_table* m_xkbComposeTable;

    /** Character compose state. */
    xkb_compose_state* m_xkbComposeState;

    /** Current modifiers mask (KeyEvent flags). */
    uint32_t m_modifiers;
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_KEYMAPXKBV1_HPP_*/
