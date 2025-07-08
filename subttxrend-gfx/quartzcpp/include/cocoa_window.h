/**
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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
 */
/**
 * @file cocoa_window.h
 * Subtec Simulator Window
 */

#ifndef __SUBTEC_COCOA_WINDOW_H
#define __SUBTEC_COCOA_WINDOW_H

#include <glib.h>
#include <OpenGL/gl.h>

namespace subttxrend
{
namespace gfx
{

/**
 * @brief Create and Run Cocoa Window object.
 *
 * Note: This is a blocking call
 *
 * @param width     Width of the video that will be displayed in the window.
 * @param height    Height of the video that will be displayed in the window.
 *
 * @return int      0 on success when the window closes.
 */
int createAndRunCocoaWindow(int width, int height);

/**
 * @brief Set the Simulator Window Title.
 *
 * @param title     Title to be set
 */
void setSimulatorWindowTitle( const char *title );

/**
 * @brief Set the Subtitle Data object
 *
 * @param data      Pointer to the subtitle bitmap data.
 * @param size      Size of the subtitle bitmap data (typically width * height * bytes per pixel).
 * @param width     Width of the subtitle bitmap image.
 * @param height    Height of the subtitle bitmap image.
 */
void setSubtitleData(uint8_t * data, unsigned int size, int width, int height);

/**
 * @brief Mute the subtitles (i.e. stop them being displayed)
 */
void muteSubtitles();

} // namespace gfx
} // namespace subttxrend


#endif //__SUBTEC_COCOA_WINDOW_H
