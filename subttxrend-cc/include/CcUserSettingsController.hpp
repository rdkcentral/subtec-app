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
    // Merges the active user override onto a caller-supplied colour without touching
    // embeddedColor. Used for transient/mid-row styling that must not become the new
    // embedded default (unlike onEmbeddedValue()).
    uint32_t applyOverride(uint32_t streamColor) const
    {
        uint32_t resultColor = usingUserDefinedColor ? (userDefinedColor & 0x00ffffff) : (streamColor & 0x00ffffff);

        if(usingUserDefinedOpacity)
            setOpacity(resultColor, userDefinedOpacity);
        else
            resultColor |= (streamColor & 0xff000000);

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

    // Merges the active user override onto a caller-supplied value without touching
    // embeddedValue. Used for transient/mid-row styling that must not become the new
    // embedded default (unlike onEmbeddedValue()).
    EmbeddedType applyOverride(EmbeddedType streamValue) const
    {
        if(usingUserDefinedValue)
            return static_cast<EmbeddedType>(userDefinedValue);
        else
            return streamValue;
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

    // Applies any active user colour override onto transient/stream-provided styling
    // (e.g. CEA-608 mid-row/PAC style codes) without recording it as the new embedded
    // default colour.
    PenColor applyPenColorOverride(PenColor streamColor) const
    {
        PenColor color;

        color.fg_color = textColorSettings.applyOverride(streamColor.fg_color);
        color.bg_color = textBgColorSettings.applyOverride(streamColor.bg_color);
        color.edge_color = textEdgeColorSettings.applyOverride(streamColor.edge_color);

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

    // Applies any active user overrides onto transient/stream-provided pen attributes
    // (e.g. CEA-608 mid-row/PAC style codes) without recording them as the new embedded
    // defaults - only the fields tracked by *Settings below are ever overridable.
    PenAttributes applyPenAttributesOverride(PenAttributes streamAttrs) const
    {
        PenAttributes result = streamAttrs;

        result.pen_color = applyPenColorOverride(streamAttrs.pen_color);
        result.pen_size = penSizeSettings.applyOverride(streamAttrs.pen_size);
        result.edge_type = penEdgeSettings.applyOverride(streamAttrs.edge_type);
        result.italics = fontItalicsSettings.applyOverride(streamAttrs.italics);
        result.underline = fontUnderlineSettings.applyOverride(streamAttrs.underline);
        result.font_tag = fontStyleSettings.applyOverride(streamAttrs.font_tag);

        return result;
    }

    WindowAttributes getWindowAttributes() const
    {
        WindowAttributes base = embeddedWindowAttributes;

        base.border_type = windowBorderTypeSettings.getValue();
        base.border_color = windowBorderColorSettings.getValue();
        base.fill_color = windowFillColorSettings.getValue();

        return base;
    }

    // Applies any active user overrides onto transient/stream-provided window attributes
    // (e.g. CEA-608 background fill changes) without recording them as the new embedded
    // defaults.
    WindowAttributes applyWindowAttributesOverride(WindowAttributes streamAttrs) const
    {
        WindowAttributes result = streamAttrs;

        result.border_type = windowBorderTypeSettings.applyOverride(streamAttrs.border_type);
        result.border_color = windowBorderColorSettings.applyOverride(streamAttrs.border_color);
        result.fill_color = windowFillColorSettings.applyOverride(streamAttrs.fill_color);

        return result;
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
