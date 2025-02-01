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


#ifndef WAYLANDCPP_KEYMAP_HPP_
#define WAYLANDCPP_KEYMAP_HPP_

#include <memory>

#include "KeyEvent.hpp"

namespace waylandcpp
{

class Keymap;

/** Pointer - Keymap. */
typedef std::shared_ptr<Keymap> KeymapPtr;

/**
 * Keymap interface.
 */
class Keymap
{
    Keymap(const Keymap&) = delete;
    Keymap& operator=(const Keymap&) = delete;

public:
    /**
     * Constructor.
     */
    Keymap() = default;

    /**
     * Destructor.
     */
    virtual ~Keymap() = default;

    /**
     * Processes key notification.
     *
     * @param key
     *      key that produced the event
     * @param state
     *      physical state of the key
     *
     * @return
     *      Key event descriptor.
     */
    virtual KeyEvent processKey(uint32_t key,
                                uint32_t state) = 0;

    /**
     * Processes modifiers notification.
     *
     * @param mods_depressed
     *      depressed modifiers
     * @param mods_latched
     *      latched modifiers
     * @param mods_locked
     *      locked modifiers
     * @param group
     *      keyboard layout
     */
    virtual void processModifiers(uint32_t mods_depressed,
                                  uint32_t mods_latched,
                                  uint32_t mods_locked,
                                  uint32_t group) = 0;
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_KEYMAP_HPP_*/
