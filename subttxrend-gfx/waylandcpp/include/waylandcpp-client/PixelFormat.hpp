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


#ifndef WAYLANDCPP_PIXELFORMAT_HPP_
#define WAYLANDCPP_PIXELFORMAT_HPP_

#include "Types.hpp"

#include <string>

namespace waylandcpp
{

/**
 * Pixel formats.
 */
enum class PixelFormat
{
    ARGB8888, //!< ARGB8888
    XRGB8888, //!< XRGB8888

    INVALID  //!< INVALID
};

/**
 * Pixel format - utilities.
 */
class PixelFormatUtils
{
    PixelFormatUtils() = delete;

public:
    /**
     * Returns number of bytes per pixel.
     *
     * @param format
     *      Pixel format.
     *
     * @return
     *      Number of bytes per pixel for given format.
     */
    static uint32_t getBytesPerPixel(PixelFormat format);

    /**
     * Converts to native (wayland) format.
     *
     * @param format
     *      Pixel format.
     *
     * @return
     *      Value representing native format.
     */
    static uint32_t toNativeFormat(PixelFormat format);

    /**
     * Converts from native format.
     *
     * @param nativeFormat
     *      Value representing native format.
     *
     * @return
     *      Pixel format (may be INVALID).
     */
    static PixelFormat fromNativeFormat(uint32_t nativeFormat);

    /**
     * Returns string desription of the format.
     *
     * @param format
     *      Pixel format.
     *
     * @return
     *      String description of the format.
     */
    static const std::string& getFormatString(PixelFormat format);
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_PIXELFORMAT_HPP_*/
