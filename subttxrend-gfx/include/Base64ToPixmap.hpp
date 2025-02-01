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

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "Types.hpp"

namespace subttxrend
{
namespace gfx
{

using PngCallback = std::function<void(const std::uint8_t* buffer, std::size_t bufferSize)>;

std::unique_ptr<gfx::Bitmap> base64toPixmap(const std::string& base64txt,
                                            bool applyRGBAlphaPremult,
                                            PngCallback callback = nullptr);

} // namespace gfx
} // namespace subttxrend
