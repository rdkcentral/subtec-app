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


#include "PixelFormat.hpp"

#include <wayland-client.h>
#include <stdexcept>

namespace waylandcpp
{

uint32_t PixelFormatUtils::getBytesPerPixel(PixelFormat format)
{
    switch (format)
    {
    case PixelFormat::ARGB8888:
    case PixelFormat::XRGB8888:
        return 4;
    default:
        throw std::invalid_argument("Unsupported format");
    }
}

uint32_t PixelFormatUtils::toNativeFormat(PixelFormat format)
{
    switch (format)
    {
    case PixelFormat::ARGB8888:
        return WL_SHM_FORMAT_ARGB8888;
    case PixelFormat::XRGB8888:
        return WL_SHM_FORMAT_XRGB8888;
    default:
        throw std::invalid_argument("Unsupported format");
    }
}

PixelFormat PixelFormatUtils::fromNativeFormat(uint32_t nativeFormat)
{
    switch (nativeFormat)
    {
    case WL_SHM_FORMAT_ARGB8888:
        return PixelFormat::ARGB8888;
    case WL_SHM_FORMAT_XRGB8888:
        return PixelFormat::XRGB8888;
    default:
        return PixelFormat::INVALID;
    }
}

const std::string& PixelFormatUtils::getFormatString(PixelFormat format)
{
    switch (format)
    {
    case PixelFormat::ARGB8888:
    {
        static const std::string formatString("ARGB8888");
        return formatString;
    }
    case PixelFormat::XRGB8888:
    {
        static const std::string formatString("XRGB8888");
        return formatString;
    }
    default:
    {
        static const std::string formatString("INVALID");
        return formatString;
    }
    }
}

} // namespace waylandcpp
