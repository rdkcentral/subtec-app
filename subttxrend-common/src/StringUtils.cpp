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


#include "StringUtils.hpp"

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <string>
#include <memory>

namespace subttxrend
{
namespace common
{

const int StringUtils::SAFE_CHAR_MIN =
        std::numeric_limits<unsigned char>::min();
const int StringUtils::SAFE_CHAR_MAX =
        std::numeric_limits<unsigned char>::max();

std::string StringUtils::trimBegin(const std::string& value)
{
    std::string trimmed = value;

    trimmed.erase(trimmed.begin(),
            std::find_if(trimmed.begin(), trimmed.end(),
                    std::not1(std::ptr_fun<int, bool>(isSpace))));

    return trimmed;
}

std::string StringUtils::trimEnd(const std::string& value)
{
    std::string trimmed = value;

    trimmed.erase(
            std::find_if(trimmed.rbegin(), trimmed.rend(),
                    std::not1(std::ptr_fun<int, bool>(isSpace))).base(),
            trimmed.end());

    return trimmed;
}

std::string StringUtils::trim(const std::string& value)
{
    std::string trimmed = value;

    trimmed.erase(trimmed.begin(),
            std::find_if(trimmed.begin(), trimmed.end(),
                    std::not1(std::ptr_fun<int, bool>(isSpace))));

    trimmed.erase(
            std::find_if(trimmed.rbegin(), trimmed.rend(),
                    std::not1(std::ptr_fun<int, bool>(isSpace))).base(),
            trimmed.end());

    return trimmed;
}

std::string StringUtils::format(const char* format,
                                ...)
{
    std::va_list arguments;
    va_start(arguments, format);

    std::string buf = vformat(format, arguments);

    va_end(arguments);

    return buf;
}

std::string StringUtils::vformat(const char *format,
                                 std::va_list arguments)
{
    static const std::size_t STACK_BUFFER_SIZE = 1024;
    static const std::size_t MAX_BUFFER_SIZE = 32 * 1024;

    char stackBuffer[STACK_BUFFER_SIZE];
    std::unique_ptr<char[]> dynamicBuffer;

    char* currentBuffer = &stackBuffer[0];
    size_t currentSize = sizeof(stackBuffer);

    for (;;)
    {
        std::va_list argumentsCopy;

        // Try to vsnprintf into our buffer.
        va_copy(argumentsCopy, arguments);
        int needed = vsnprintf(currentBuffer, currentSize, format, arguments);
        va_end(argumentsCopy);

        // NB. C99 (which modern Linux and OS X follow) says vsnprintf
        // failure returns the length it would have needed.  But older
        // glibc and current Windows return -1 for failure, i.e., not
        // telling us how much was needed.

        if ((needed <= static_cast<int>(currentSize)) && (needed >= 0))
        {
            // success
            return std::string(currentBuffer, (size_t) needed);
        }
        if (currentSize >= MAX_BUFFER_SIZE)
        {
            currentBuffer[currentSize - 1] = 0;
            return std::string(currentBuffer, (size_t) currentSize);
        }

        // prepare larger dynamic buffer when needed
        currentSize = (needed > 0) ? (needed + 1) : (currentSize * 2);

        dynamicBuffer.reset(new char[currentSize]);
        currentBuffer = dynamicBuffer.get();
    }
}

} // namespace common
} // namespace subttxrend
