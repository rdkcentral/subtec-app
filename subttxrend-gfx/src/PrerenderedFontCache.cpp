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


#include "PrerenderedFontImpl.hpp"

#include <map>
#include <string>
#include <utility>

#include "FontStripImpl.hpp"

namespace subttxrend
{
namespace gfx
{

std::shared_ptr<PrerenderedFont> PrerenderedFontCache::getFont(const std::string& fontName, int faceHeight, bool strictHeight, bool italics)
{
    auto fontKey = std::make_tuple(fontName, Height{faceHeight, strictHeight}, italics);
    auto iter = m_fontPathAndSizeToFont.find(fontKey);
    
    if (iter == m_fontPathAndSizeToFont.end())
    {
        std::string fontPath = FontStripImpl::findFontFile(fontName.c_str());
        auto newFont = std::make_shared<PrerenderedFontImpl>(fontPath.c_str(), faceHeight, strictHeight, italics);
        m_fontPathAndSizeToFont[fontKey] = newFont;
        return newFont;
    }
    else
    {
        return iter->second;
    }
}

void PrerenderedFontCache::clear()
{
    m_fontPathAndSizeToFont.clear();
}


}   // namespace gfx
}   // namespace subttxrend
