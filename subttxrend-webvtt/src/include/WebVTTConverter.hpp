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
#pragma once

#include <Constants.hpp>
#include <WebVTTAttributes.hpp>
#include <WebVTTCue.hpp>
#include <WebVTTConfig.hpp>

namespace subttxrend {
namespace webvttengine {

class Converter {
    //Apply padding for overscan
    inline int calculateDisplayDimension(int dimension, int paddingVmH) 
    { 
        return (int)((((float)dimension) * (100.0f - (hundredthsToV(paddingVmH) * 2.0f))) / 100.0f);
    }
public:
    Converter(int width, int height)
    {
        m_displayWidth = calculateDisplayDimension(width, constants::kScreenPaddingVmH);
        m_displayHeight = calculateDisplayDimension(height, constants::kScreenPaddingVmH);
    }

    Converter(int width, int height, const WebVTTConfig &config, const WebVTTAttributes &attributes)
    {
        m_defaultLineHeight = config.lineHeightH;
        m_defaultFontHeight = config.fontHeightH;
        setAttributes(attributes);
        horiz_padding_em_tenths_ = config.horizontalPaddingEm;
        vertical_padding_em_tenths_ = config.verticalPaddingEm;
        m_displayWidth = calculateDisplayDimension(width, config.screenPadding);
        m_displayHeight = calculateDisplayDimension(height, config.screenPadding);
        screenPadding = config.screenPadding;
    }

    void SetDimensions(int width, int height) { m_displayWidth = width; m_displayHeight = height; }

    inline int height()                           { return m_displayHeight; }
    inline int width()                           { return m_displayWidth; }
    inline int screenPaddingWidthPixels()      { return vwToPixels(hundredthsToV(screenPadding)); }
    inline int screenPaddingHeightPixels()      { return vhToPixels(hundredthsToV(screenPadding)); }
    inline int vwToWidthPixels(int width)      { return vwToPixels(hundredthsToV(width)); }
    inline int vhToHeightPixels(int height)    { return vhToPixels(hundredthsToV(height)); }
    inline int fontSizePixels()                { return vhToPixels(hundredthsToV(m_fontHeight)); }
    inline int lineHeightPixels()              { return vhToPixels(hundredthsToV(m_lineHeight)); }
    inline int lineHeightVh()              { return m_lineHeight; }
    inline int horizontalPadding() {
        return vhToPixels(hundredthsToV(((float)m_fontHeight) * (horiz_padding_em_tenths_ / 10.0f)));
    }
    inline int verticalPadding() {
        return vhToPixels(hundredthsToV(((float)m_fontHeight) * (vertical_padding_em_tenths_ / 10.0f)));
    }
    int getXForTextBox(int line_width_px, WebVTTCue::AlignType align, int position_px) {
        int text_box_x = 0;

        switch (align) {
        case WebVTTCue::AlignType::kLeft:
        case WebVTTCue::AlignType::kStart:
            text_box_x = position_px;
            break;
        case WebVTTCue::AlignType::kRight:
        case WebVTTCue::AlignType::kEnd:
            text_box_x = position_px - line_width_px;
            break;
        case WebVTTCue::AlignType::kCenter:
            text_box_x = position_px - (line_width_px / 2);
            break;
        }

        return text_box_x;
    }

    inline bool isTopPositioningSet()       { return m_isTopPositioningSet; }
    inline int topPositioningPixels()       { return vhToPixels(hundredthsToV(m_topPositioning)); }

    void setAttributes(const WebVTTAttributes &attributes)
    {
        m_attributes.update(attributes);

        if (m_attributes.isSet(WebVTTAttributes::AttributeType::FONT_SIZE))
        {
            uint32_t fontSizeAttribute = m_attributes.getInteger(WebVTTAttributes::AttributeType::FONT_SIZE);
            if (fontSizeAttribute < constants::kNumberOfFonts)
            {
                m_lineHeight = constants::kLineHeight[fontSizeAttribute];
                m_fontHeight = constants::kFontHeight[fontSizeAttribute];
                m_topPositioning = constants::kTopPositioning[fontSizeAttribute];
                m_isTopPositioningSet = true;
            }
            else
            {
                m_lineHeight = m_defaultLineHeight;
                m_fontHeight = m_defaultFontHeight;
                m_topPositioning = 0;
                m_isTopPositioningSet = false;
            }
        }
        else
        {
            m_lineHeight = m_defaultLineHeight;
            m_fontHeight = m_defaultFontHeight;
            m_topPositioning = 0;
            m_isTopPositioningSet = false;
        }
    }

private:
    template<typename T>
    inline int vwToPixels(T vw) const { return (int)(((float)m_displayWidth / 100.0f) * vw); }

    template<typename T>
    inline int vhToPixels(T vh) const { return (int)(((float)m_displayHeight / 100.0f) * vh); }

    inline float hundredthsToV(float hundredths) const { return hundredths / 100.0f; }
    int m_displayWidth {1280};
    int m_displayHeight {720};

    int        m_defaultLineHeight {constants::kDefaultLineHeight};  //vh
    int        m_defaultFontHeight {constants::kDefaultFontHeight};  //vh
    int        m_lineHeight {constants::kDefaultLineHeight};  //vh
    int        m_fontHeight {constants::kDefaultFontHeight};  //vh
    bool       m_isTopPositioningSet {false};
    int        m_topPositioningSet {0};
    int        m_topPositioning {0};
    int        screenPadding {constants::kScreenPaddingVmH};
    float      horiz_padding_em_tenths_ {constants::kDefaultHorizPaddingEmTenths};
    float      vertical_padding_em_tenths_ {constants::kDefaultVerticalPaddingEmTenths};
    WebVTTAttributes m_attributes;
};
}
}

