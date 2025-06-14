/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
 * @file  TextStyleAttributes.h
 *
 * @brief This file provides class and other definition related to subtitle text attributes
 *
 */

#pragma once

#include <string.h>
#include <array>
#include "SubtecAttribute.hpp"
#include <map>

    /**
     * @enum FontSize
     * @brief Available Fontsize
     */
    typedef enum FontSize {
        FONT_SIZE_EMBEDDED = -1,            /* Corresponds to Font size of auto */
        FONT_SIZE_SMALL,
        FONT_SIZE_STANDARD,
        FONT_SIZE_LARGE,
        FONT_SIZE_EXTRALARGE,
        FONT_SIZE_MAX
    } FontSize;

    /**
     *  @enum FontStyle
     *  @brief Available Fontstyles
     */
    typedef enum FontStyle {
        FONT_STYLE_EMBEDDED = -1,
        FONT_STYLE_DEFAULT,
        FONT_STYLE_MONOSPACED_SERIF,
        FONT_STYLE_PROPORTIONAL_SERIF,
        FONT_STYLE_MONOSPACED_SANSSERIF,
        FONT_STYLE_PROPORTIONAL_SANSSERIF,
        FONT_STYLE_CASUAL,
        FONT_STYLE_CURSIVE,
        FONT_STYLE_SMALL_CAPITALS,
        FONT_STYLE_MAX
    } FontStyle;

    /**
     *  @enum SupportedColors
     *  @brief Supported colors
     */
    typedef enum SupportedColors {
        COLOR_EMBEDDED = 0xFF000000,
        COLOR_BLACK = 0x00000000,
        COLOR_WHITE = 0x00FFFFFF,
        COLOR_RED = 0x00FF0000,
        COLOR_GREEN = 0x0000FF00,
        COLOR_BLUE = 0x000000FF,
        COLOR_YELLOW = 0x00FFFF00,
        COLOR_MAGENTA = 0x00FF00FF,
        COLOR_CYAN = 0x0000FFFF
    } SupportedColors;

    const std::map< std::string, SupportedColors> ColorMapTable {
        { "black", COLOR_BLACK},
        { "white", COLOR_WHITE},
        { "red", COLOR_RED},
        { "green", COLOR_GREEN},
        { "blue", COLOR_BLUE},
        { "yellow", COLOR_YELLOW},
        { "magenta", COLOR_MAGENTA},
        { "cyan", COLOR_CYAN},
        { "auto", COLOR_EMBEDDED}
    };

    /**
     *  @enum EdgeType
     *  @brief Available Edge Types
     */
    typedef enum EdgeType {
        EDGE_TYPE_EMBEDDED = -1,
        EDGE_TYPE_NONE,
        EDGE_TYPE_RAISED,
        EDGE_TYPE_DEPRESSED,
        EDGE_TYPE_UNIFORM,
        EDGE_TYPE_SHADOW_LEFT,
        EDGE_TYPE_SHADOW_RIGHT,
        EDGE_TYPE_MAX
    } EdgeType;

    /**
     *  @enum Opacity
     *  @brief Available Opacity options
     */
    typedef enum Opacity {
        OPACITY_EMBEDDED = -1,
        OPACITY_SOLID,
        OPACITY_FLASHING,
        OPACITY_TRANSLUCENT,
        OPACITY_TRANSPARENT,
        OPACITY_MAX
    } Opacity;

    /**
     * @enum AttribPosInArray
     * @brief Provides the indexing postion in array for attributes
     */
    typedef enum AttribPosInArray {
        FONT_COLOR_ARR_POSITION = 0,
        BACKGROUND_COLOR_ARR_POSITION,
        FONT_OPACITY_ARR_POSITION,
        BACKGROUND_OPACITY_ARR_POSITION,
        FONT_STYLE_ARR_POSITION,
        FONT_SIZE_ARR_POSITION,
        FONT_ITALIC_ARR_POSITION,
        FONT_UNDERLINE_ARR_POSITION,
        BORDER_TYPE_ARR_POSITION,
        BORDER_COLOR_ARR_POSITION,
        WIN_COLOR_ARR_POSITION,
        WIN_OPACITY_ARR_POSITION,
        EDGE_TYPE_ARR_POSITION,
        EDGE_COLOR_ARR_POSITION
    } AttribPosInArray;
