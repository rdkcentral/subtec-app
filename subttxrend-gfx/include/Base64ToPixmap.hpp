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

/**
 * Decodes a base64 encoded PNG image (RGBA or palette with transparency).
 *
 * The result is 32-bit ARGB in native (little endian) byte order, i.e. bytes
 * B, G, R, A in memory, as used by Bitmap.
 *
 * @note Applies a subtitle workaround: if all four corners have the same
 *       non-transparent colour, every pixel of that colour is made fully
 *       transparent. Use base64toPixmapKeepBackground() for images whose
 *       colours must be kept, such as Engine background images.
 *
 * @param base64txt
 *      Base64 encoded PNG data.
 * @param applyRGBAlphaPremult
 *      True to premultiply colour by alpha. Pixmaps use straight alpha, so
 *      pass false for images drawn through the gfx engine.
 * @param callback
 *      Optional callback receiving the decoded (still PNG encoded) data.
 *
 * @return
 *      Decoded bitmap, nullptr on error.
 */
std::unique_ptr<gfx::Bitmap> base64toPixmap(const std::string& base64txt,
                                            bool applyRGBAlphaPremult,
                                            PngCallback callback = nullptr);

/**
 * Decodes a base64 encoded PNG image without the background workaround.
 *
 * Same as base64toPixmap(), except all pixels keep their decoded colours.
 */
std::unique_ptr<gfx::Bitmap> base64toPixmapKeepBackground(const std::string& base64txt,
                                                          bool applyRGBAlphaPremult,
                                                          PngCallback callback = nullptr);

} // namespace gfx
} // namespace subttxrend
