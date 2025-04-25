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
 * @file SubtecConnector.h
 *
 * @brief Interface header for libsubtec_connector
 *
 */

#ifndef __SUBTEC_CONNECTOR_H__
#define __SUBTEC_CONNECTOR_H__


/* Closed Captioning Color definition */

#define GSW_CC_EMBEDDED_COLOR (0xff000000)
#define GSW_CC_COLOR(r,g,b)  ( (((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((b) & 0xFF) )

#define GSW_MAX_CC_COLOR_NAME_LENGTH 32

/**
 * @struct gsw_CcColor
 * @brief Structure to hold color information for CC
 */

typedef struct gsw_CcColor {
    int rgb;
    char name[GSW_MAX_CC_COLOR_NAME_LENGTH];
} gsw_CcColor;
// Maximum number of CC color capability values
#define GSW_CC_COLOR_MAX 32


/**
 * @enum gsw_CcType
 * @brief Closed Captioning type
 */
typedef enum gsw_CcType {
    GSW_CC_TYPE_ANALOG,
    GSW_CC_TYPE_DIGITAL,
    GSW_CC_TYPE_MAX
} gsw_CcType;


/**
 * @enum gsw_CcOpacity
 * @brief Closed Captioning Opacity
 */
typedef enum gsw_CcOpacity {
    GSW_CC_OPACITY_EMBEDDED = -1,
    GSW_CC_OPACITY_SOLID,
    GSW_CC_OPACITY_FLASHING,
    GSW_CC_OPACITY_TRANSLUCENT,
    GSW_CC_OPACITY_TRANSPARENT,
    GSW_CC_OPACITY_MAX
} gsw_CcOpacity;

/**
 * @enum gsw_CcFontSize
 * @brief Closed caption Fontsize
 */
typedef enum gsw_CcFontSize {
    GSW_CC_FONT_SIZE_EMBEDDED = -1,
    GSW_CC_FONT_SIZE_SMALL,
    GSW_CC_FONT_SIZE_STANDARD,
    GSW_CC_FONT_SIZE_LARGE,
    GSW_CC_FONT_SIZE_EXTRALARGE,
    GSW_CC_FONT_SIZE_MAX
} gsw_CcFontSize;


static const char *CCSupportedColors_strings[] = {
    "BLACK",                /**< CCColor_BLACK,            */
    "WHITE",                /**< CCColor_WHITE,            */
    "RED",                  /**< CCColor_RED,              */
    "GREEN",                /**< CCColor_GREEN,            */
    "BLUE",                 /**< CCColor_BLUE,             */
    "YELLOW",               /**< CCColor_YELLOW,           */
    "MAGENTA",              /**< CCColor_MAGENTA,          */
    "CYAN",                 /**< CCColor_CYAN,             */
    "AUTO",                 /**< CCColor_AUTO,             */
};

static unsigned long CCSupportedColors[] = {
    0x00000000,             /**< CCColor_BLACK,            */
    0x00ffffff,             /**< CCColor_WHITE,            */
    0x00FF0000,             /**< CCColor_RED,              */
    0x0000FF00,             /**< CCColor_GREEN,            */
    0x000000FF,             /**< CCColor_BLUE,             */
    0x00FFFF00,             /**< CCColor_YELLOW,           */
    0x00FF00FF,             /**< CCColor_MAGENTA,          */
    0x0000FFFF,             /**< CCColor_CYAN,             */
    0xFF000000,             /**< CCColor_AUTO,             */
};


/* Closed Captioning Font Style */
#define GSW_CC_MAX_FONT_NAME_LENGTH 128
//#define GSW_CC_FONT_STYLE_EMBEDDED "EMBEDDED"
typedef char gsw_CcFontStyle[GSW_CC_MAX_FONT_NAME_LENGTH];

#define GSW_CC_FONT_STYLE_EMBEDDED 		"embedded"  //"EMBEDDED"
#define GSW_CC_FONT_STYLE_DEFAULT 		"Default"   //"DEFAULT"
#define GSW_CC_FONT_STYLE_MONOSPACED_SERIF	"MonospacedSerif"   //"MONOSPACED_SERIF"
#define GSW_CC_FONT_STYLE_PROPORTIONAL_SERIF	"ProportionalSerif" //"PROPORTIONAL_SERIF"
#define GSW_CC_FONT_STYLE_MONOSPACED_SANSSERIF	"MonospacedSansSerif"   //"MONOSPACED_SANSSERIF"
#define GSW_CC_FONT_STYLE_PROPORTIONAL_SANSSERIF  "ProportionalSansSerif"   //"PROPORTIONAL_SANSSERIF"
#define GSW_CC_FONT_STYLE_CASUAL		"Casual"    //"CASUAL"
#define GSW_CC_FONT_STYLE_CURSIVE		"Cursive"   //"CURSIVE"
#define GSW_CC_FONT_STYLE_SMALL_CAPITALS	"SmallCapital"  //"SMALL_CAPITALS"


/**
 * @enum gsw_CcTextStyle
 * @brief Closed captioning text styles.
 */
typedef enum gsw_CcTextStyle {
    GSW_CC_TEXT_STYLE_EMBEDDED_TEXT = -1,
    GSW_CC_TEXT_STYLE_FALSE,
    GSW_CC_TEXT_STYLE_TRUE,
    GSW_CC_TEXT_STYLE_MAX
} gsw_CcTextStyle;


/**
 * @enum gsw_CcBorderType
 * @brief Window Border type
 */
typedef enum gsw_CcBorderType {
    GSW_CC_BORDER_TYPE_EMBEDDED = -1,
    GSW_CC_BORDER_TYPE_NONE,
    GSW_CC_BORDER_TYPE_RAISED,
    GSW_CC_BORDER_TYPE_DEPRESSED,
    GSW_CC_BORDER_TYPE_UNIFORM,
    GSW_CC_BORDER_TYPE_SHADOW_LEFT,
    GSW_CC_BORDER_TYPE_SHADOW_RIGHT,
    GSW_CC_BORDER_TYPE_MAX
} gsw_CcBorderType;

/**
 * @enum gsw_CcEdgeType
 * @brief Font Edge type
 */
typedef enum gsw_CcEdgeType {
    GSW_CC_EDGE_TYPE_EMBEDDED = -1,
    GSW_CC_EDGE_TYPE_NONE,
    GSW_CC_EDGE_TYPE_RAISED,
    GSW_CC_EDGE_TYPE_DEPRESSED,
    GSW_CC_EDGE_TYPE_UNIFORM,
    GSW_CC_EDGE_TYPE_SHADOW_LEFT,
    GSW_CC_EDGE_TYPE_SHADOW_RIGHT,
    GSW_CC_EDGE_TYPE_MAX
} gsw_CcEdgeType;

/**
 * @enum gsw_CcAttributes
 * @brief Closed Captioning Attributes
 */
typedef struct gsw_CcAttributes {
    gsw_CcColor charBgColor;		/**< character background color */
    gsw_CcColor charFgColor;    	/**< character foreground color */
    gsw_CcColor winColor;       	/**< window color */
    gsw_CcOpacity charBgOpacity;    	/**< background opacity */
    gsw_CcOpacity charFgOpacity;    	/**< foreground opacity */
    gsw_CcOpacity winOpacity;   	/**< window opacity */
    gsw_CcFontSize fontSize;    	/**< font size */
    gsw_CcFontStyle fontStyle;  	/**< font style */
    gsw_CcTextStyle fontItalic; 	/**< italicized font */
    gsw_CcTextStyle fontUnderline;  	/**< underlined font */
    gsw_CcBorderType borderType;    	/**< window border type */
    gsw_CcColor borderColor;    	/**< window border color */
    gsw_CcEdgeType edgeType;    	/**< font edge type */
    gsw_CcColor edgeColor;      	/**< font edge color */

} gsw_CcAttributes;

/**
 * @enum gsw_CcAttribType
 * @brief type of attributes
 */
typedef enum gsw_CcAttribType {
    GSW_CC_ATTRIB_FONT_COLOR = 0x0001,
    GSW_CC_ATTRIB_BACKGROUND_COLOR = 0x0002,
    GSW_CC_ATTRIB_FONT_OPACITY = 0x0004,
    GSW_CC_ATTRIB_BACKGROUND_OPACITY = 0x0008,
    GSW_CC_ATTRIB_FONT_STYLE = 0x0010,
    GSW_CC_ATTRIB_FONT_SIZE = 0x0020,
    GSW_CC_ATTRIB_FONT_ITALIC = 0x0040,
    GSW_CC_ATTRIB_FONT_UNDERLINE = 0x0080,
    GSW_CC_ATTRIB_BORDER_TYPE = 0x0100,
    GSW_CC_ATTRIB_BORDER_COLOR = 0x0200,
    GSW_CC_ATTRIB_WIN_COLOR = 0x0400,
    GSW_CC_ATTRIB_WIN_OPACITY = 0x0800,
    GSW_CC_ATTRIB_EDGE_TYPE = 0x1000,
    GSW_CC_ATTRIB_EDGE_COLOR = 0x2000,
    GSW_CC_ATTRIB_MAX
} gsw_CcAttribType;

#endif //__SUBTEC_CONNECTOR_H__
