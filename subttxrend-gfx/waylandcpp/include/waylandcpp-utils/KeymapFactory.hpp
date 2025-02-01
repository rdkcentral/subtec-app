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


#ifndef WAYLANDCPP_KEYMAP_FACTORY_HPP_
#define WAYLANDCPP_KEYMAP_FACTORY_HPP_

#include "Keymap.hpp"

namespace waylandcpp
{

/**
 * Keymap factory.
 */
class KeymapFactory
{
    KeymapFactory() = delete;

public:
    /**
     * Creates a keymap.
     *
     * @param format
     *      keymap format
     * @param fd
     *      keymap file descriptor
     * @param size
     *      keymap size, in bytes
     *
     * @return
     *      Created keymap or null if keymap cannot be created.
     */
    static KeymapPtr createKeymap(uint32_t format,
                                  int32_t fd,
                                  uint32_t size);
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_KEYMAP_FACTORY_HPP_*/
