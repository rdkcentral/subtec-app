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

#include <string>

namespace subttxrend
{
namespace ttmlengine
{

/**
 * Safe std::isspace.
 *
 * @param c
 *      Character to check.
 *
 * @retval true
 *      Character is whitespace.
 * @retval false.
 *      Character is not whitespace.
 */
bool isSpace(int c);

/**
 * Trims whitespace characters.
 *
 * @param s
 *      String to trim.
 * @return
 *      Trimmed string.
 */
std::string trimWhitespace(const std::string &s);

} // namespace subttxrend
} // namespace ttmlengine
