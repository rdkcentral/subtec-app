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

#include <limits>
#include <string>
#include <sstream>

namespace subttxrend
{
namespace webvttengine
{

namespace constants
{
static constexpr int    kScreenPaddingVmH = 100;
static constexpr int    kCueAutoSetting = std::numeric_limits<int>::max();
static constexpr int    kDefaultLineHeight = 600;  //vh
static constexpr int    kDefaultFontHeight = 533;  //vh
static constexpr int    kDefaultHorizPaddingEmTenths = 3;
static constexpr int    kDefaultVerticalPaddingEmTenths = 3;
static constexpr int    kNumberOfFonts = 4;
static constexpr int    kFontHeight[4] = {315, 463, 741, 926}; // vh - 34px, 50px, 80px, 100px at 1080px height
static constexpr int    kLineHeight[4] = {354, 521, 833, 1042}; // vh
static constexpr int    kTopPositioning[4] = {8056, 7963, 7315, 7130}; // vh - 870px, 860px, 790px, 770px at 1080px height
}

}
}