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
*
* Copyright 2023 Comcast Cable Communications Management, LLC
* Licensed under the Apache License, Version 2.0
*****************************************************************************/

#pragma once

#include "CcCommand.hpp"
#include "CcCommandParser.hpp"
#include "CcCommandProcessor.hpp"

#include <memory>
#include <functional>


#include "CcCommand.hpp"
#include <subttxrend/common/Logger.hpp>

namespace subttxrend
{
namespace cc
{


class ColorUserSettings
{
public:
    void onEmbeddedValue(uint32_t color)
    {
        embeddedColor = color;
    }
    void onUserDefinedColor(uint32_t color)
    {
        if(color == 0xff000000)
        {
            usingUserDefinedColor = false;
            userDefinedColor = 0;
        }
        else
        {
            usingUserDefinedColor = true;
            userDefinedColor = color;
        }
    }
    void onUserDefinedOpacity(uint32_t opacity)
    {
        if(opacity == (uint32_t)-1)
        {
            usingUserDefinedOpacity = false;
            userDefinedOpacity = 0;

        }
        else
        {
            usingUserDefinedOpacity = true;
            userDefinedOpacity = opacity;
        }
    }
    uint32_t getValue() const
    {
        uint32_t resultColor = 0;
        if(usingUserDefinedColor)
            resultColor = userDefinedColor & 0x00ffffff;
        else
            resultColor = embeddedColor & 0x00ffffff;

        if(usingUserDefinedOpacity)
            setOpacity(resultColor, userDefinedOpacity);
        else
            resultColor |= ((embeddedColor & 0xff000000));

        return resultColor;
    }
private:
    uint32_t embeddedColor{};
    uint32_t userDefinedColor{};
    uint32_t userDefinedOpacity{};

    bool usingUserDefinedColor{false};
    bool usingUserDefinedOpacity{false};

};

template<typename EmbeddedType, typename UserDefinedType, UserDefinedType useEmbeddedMarker>
class UserSettings
{
public:
    void onEmbeddedValue(EmbeddedType value)
    {
        embeddedValue = value;
    }

    void onUserDefinedValue(UserDefinedType value)
    {
        if(value == useEmbeddedMarker)
        {
            usingUserDefinedValue = false;
            userDefinedValue = {};
        }
        else
        {
            usingUserDefinedValue = true;
            userDefinedValue = value;
        }
    }

    EmbeddedType getValue() const
    {
        if(usingUserDefinedValue)
            return static_cast<EmbeddedType>(userDefinedValue);
        else
            return embeddedValue;
    }

private:
    EmbeddedType embeddedValue{};
    UserDefinedType userDefinedValue{};
    bool usingUserDefinedValue{false};
};

using PenSizeSettings = UserSettings<PenSize, uint32_t, (uint32_t)-1>;
using PenEdgeSettings = UserSettings<PenEdge, uint32_t, (uint32_t)-1>;
using FontItalicsSettings = UserSettings<bool, uint32_t, (uint32_t)-1>;
using FontUnderlineSettings = UserSettings<bool, uint32_t, (uint32_t)-1>;
using FontStyleSettings = UserSettings<PenFontStyle, uint32_t, (uint32_t)-1>;
using WindowBorderTypeSettings = UserSettings<WindowBorder, uint32_t, (uint32_t)-1>;


class UserSettingsController
{
public:
    void onEmbeddedPenColor(PenColor color)
    {
        textColorSettings.onEmbeddedValue(color.fg_color);
        textBgColorSettings.onEmbeddedValue(color.bg_color);
        textEdgeColorSettings.onEmbeddedValue(color.edge_color);
    }

    void onEmbeddedPenAttributes(PenAttributes attribs)
    {
        // to store values not altered by user settings
        embeddedPenAttributes = attribs;

        penSizeSettings.onEmbeddedValue(attribs.pen_size);
        penEdgeSettings.onEmbeddedValue(attribs.edge_type);
        fontItalicsSettings.onEmbeddedValue(attribs.italics);
        fontUnderlineSettings.onEmbeddedValue(attribs.underline);
        fontStyleSettings.onEmbeddedValue(attribs.font_tag);
    }

    void onEmbeddedWindowAttributes(WindowAttributes attribs)
    {
        // to store values not altered by user settings
        embeddedWindowAttributes = attribs;

        windowBorderTypeSettings.onEmbeddedValue(attribs.border_type);
        windowBorderColorSettings.onEmbeddedValue(attribs.border_color);
        windowFillColorSettings.onEmbeddedValue(attribs.fill_color);
    }

    void onUserDefinedTextColor(uint32_t value)             {       textColorSettings.onUserDefinedColor(value);            }

    void onUserDefinedTextBgColor(uint32_t value)           {       textBgColorSettings.onUserDefinedColor(value);          }

    void onUserDefinedTextBgOpacity(uint32_t value)         {       textBgColorSettings.onUserDefinedOpacity(value);        }

    void onUserDefinedTextEdgeColor(uint32_t value)         {       textEdgeColorSettings.onUserDefinedColor(value);        }

    void onUserDefinedTextEdgeOpacity(uint32_t value)       {       textEdgeColorSettings.onUserDefinedOpacity(value);      }

    void onUserDefinedTextOpacity(uint32_t value)           {       textColorSettings.onUserDefinedOpacity(value);          }

    void onUserDefinedPenSize(uint32_t value)               {       penSizeSettings.onUserDefinedValue(value);              }

    void onUserDefinedPenEdge(uint32_t value)               {       penEdgeSettings.onUserDefinedValue(value);              }

    void onUserDefinedFontStyle(uint32_t value)             {       fontStyleSettings.onUserDefinedValue(value);            }

    void onUserDefinedFontItalics(uint32_t value)           {       fontItalicsSettings.onUserDefinedValue(value);          }

    void onUserDefinedFontUnderline(uint32_t value)         {       fontUnderlineSettings.onUserDefinedValue(value);        }

    void onUserDefinedWindowBorderType(uint32_t value)      {       windowBorderTypeSettings.onUserDefinedValue(value);     }

    void onUserDefinedWindowBorderColor(uint32_t value)     {       windowBorderColorSettings.onUserDefinedColor(value);    }

    void onUserDefinedWindowBorderOpacity(uint32_t value)   {       windowBorderColorSettings.onUserDefinedOpacity(value);  }

    void onUserDefinedWindowFillColor(uint32_t value)       {       windowFillColorSettings.onUserDefinedColor(value);      }

    void onUserDefinedWindowFillOpacity(uint32_t value)     {       windowFillColorSettings.onUserDefinedOpacity(value);    }

    PenColor getPenColor() const
    {
        PenColor color;

        color.fg_color = textColorSettings.getValue();
        color.bg_color = textBgColorSettings.getValue();
        color.edge_color = textEdgeColorSettings.getValue();

        return color;
    }

    PenAttributes getPenAttributes() const
    {
        PenAttributes base = embeddedPenAttributes;

        base.pen_color = getPenColor();

        base.pen_size = penSizeSettings.getValue();
        base.edge_type = penEdgeSettings.getValue();
        base.italics = fontItalicsSettings.getValue();
        base.underline = fontUnderlineSettings.getValue();
        base.font_tag = fontStyleSettings.getValue();

        return base;
    }

    WindowAttributes getWindowAttributes() const
    {
        WindowAttributes base = embeddedWindowAttributes;

        base.border_type = windowBorderTypeSettings.getValue();
        base.border_color = windowBorderColorSettings.getValue();
        base.fill_color = windowFillColorSettings.getValue();

        return base;
    }

    ColorUserSettings textColorSettings;
    ColorUserSettings textBgColorSettings;
    ColorUserSettings textEdgeColorSettings;

    PenAttributes embeddedPenAttributes{};

    PenSizeSettings penSizeSettings;
    PenEdgeSettings penEdgeSettings;
    FontItalicsSettings fontItalicsSettings;
    FontUnderlineSettings fontUnderlineSettings;
    FontStyleSettings fontStyleSettings;

    WindowAttributes embeddedWindowAttributes{};

    WindowBorderTypeSettings windowBorderTypeSettings;
    ColorUserSettings windowBorderColorSettings;
    ColorUserSettings windowFillColorSettings;

};

} // namespace cc
} // namespace subttxrend
