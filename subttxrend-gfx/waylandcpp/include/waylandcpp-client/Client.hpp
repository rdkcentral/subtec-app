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


#ifndef WAYLANDCPP_CLIENT_HPP_
#define WAYLANDCPP_CLIENT_HPP_

#include "Types.hpp"
#include "Display.hpp"

namespace waylandcpp
{

/**
 * Wayland client entry point class.
 */
class Client
{
    Client() = delete;
    ~Client() = delete;

public:
    /**
     * Connect to a Wayland display.
     *
     * @param name
     *      Name of the Wayland display to connect to
     *
     * @return
     *      Display object or nullptr on failure.
     */
    template<class DisplayInterface>
    static typename DisplayInterface::Ptr connect(const char* name)
    {
        DisplayPtr ptr = connectByName(name);

        if (!ptr)
        {
            return nullptr;
        }

        return ptr->getInterface<DisplayInterface>();
    }

    /**
     * Connect to a Wayland display.
     *
     * @param name
     *      Name of the Wayland display to connect to
     *
     * @return
     *      Display object or nullptr on failure.
     */
    static DisplayPtr connectByName(const char* name);

    /**
     * Connect to Wayland display on an already open fd.
     *
     * @param fd
     *      The fd to use for the connection
     *
     * @return
     *      Display object or nullptr on failure.
     */
    static DisplayPtr connectByFd(int fd);

};

} // namespace waylandcpp

#endif /*WAYLANDCPP_CLIENT_HPP_*/
