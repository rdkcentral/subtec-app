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

#include <subttxrend/common/Logger.hpp>
#include <subttxrend/gfx/PrerenderedFont.hpp>

#include "CcGfx.hpp"
#include "CcTextDrawer.hpp"

namespace subttxrend
{
namespace cc
{

class Window
{
    public:
        Window(std::shared_ptr<Gfx> gfx, std::shared_ptr<gfx::PrerenderedFontCache> font, WindowDefinition windef, bool enable608 = false);
        virtual ~Window();

        virtual bool changed();

        virtual void draw();
        virtual void show();
        virtual void hide();
        virtual void toggle();
        virtual void clear();
        virtual void setPenLocation(int row, int column);
        virtual void report(std::string str);
        virtual void setWindowAttributes(WindowAttributes attr);
        virtual void setPenAttributes(PenAttributes penattrs);
        virtual void setPenColor(PenColor pencolor);
        virtual void carriageReturn();
        virtual void horizontalCarriageReturn();
        virtual void backspace();
        virtual void transparentSpace(bool nonbreaking);

        void activePenAttributes(PenAttributes &penAttributes);
        void overridePenAttributes(PenAttributes penattrs, bool midRow = false);
        void setTabOffset(int offset);
        void updateRowCount(int rowCount, bool adjust);

        void ensureTextDrawer();
        virtual void update(const WindowDefinition &windef);

        bool hasFlashingText();
        void setFlashState(FlashControl state);
        bool hasText(int row);

        int ID() const
        {
            return m_def.id;
        }

        int getPriority() const
        {
            return m_def.priority;
        }

        void getDefinition(WindowDefinition &wd)
        {
            wd = m_def;
        }

        bool isVisible() const
        {
            return m_def.visible;
        }

        bool windowVisibilityChanged() const
        {
            return m_visibilityChanged;
        }

    private:
        struct Rect{
          int x;
          int y;
          int w;
          int h;
        };

        Point calculate(const ScreenInfo& screenInfo);
        Point calculateAnchorTopLeftPoint(const Dimensions& bDim);
        Dimensions calculateRects4Text(std::vector<Rect>& tdRects);
        Point justifyTextDrawer(const Point& anchorPoint, Rect& textRect, const Dimensions& windowDim);
        void drawTextDrawers(const Point& anchorPoint, std::vector<Rect>& tdRects, const Dimensions& windowDim);
        Point cursor();

        bool ignoreColumn();
        void scroll(int row, int column);

        std::shared_ptr<Gfx> m_gfx;
        std::shared_ptr<gfx::PrerenderedFontCache> m_fontCache;
        WindowDefinition m_def;
        std::vector<std::unique_ptr<TextDrawer>> m_textDrawers;
        FontGroup m_fonts;
        bool m_changed;
        bool m_608Enabled;
        bool m_visibilityChanged;

        common::Logger logger;
};

} // namespace cc
} // namespace subttxrend

