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


#ifndef SUBTTXREND_COMMON_STRINGUTILS_HPP_
#define SUBTTXREND_COMMON_STRINGUTILS_HPP_

#include <cstdarg>
#include <string>
#include <cctype>
#include <limits>
#include <algorithm>

namespace subttxrend
{
namespace common
{

/**
 * String utilities.
 */
class StringUtils
{
public:
    StringUtils() = delete;
    ~StringUtils() = delete;

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
    static bool isSpace(int c)
    {
        if ((c >= SAFE_CHAR_MIN) && (c <= SAFE_CHAR_MAX))
        {
            return std::isspace(c);
        }
        else
        {
            return false;
        }
    }

    /**
     * Safe std::tolower.
     *
     * @param c
     *      Character to convert.
     *
     * @return
     *      Converted character.
     */
    static int toLower(int c)
    {
        if ((c >= SAFE_CHAR_MIN) && (c <= SAFE_CHAR_MAX))
        {
            return std::tolower(c);
        }
        else
        {
            return c;
        }
    }

    /**
     * Converts string to lower-case.
     *
     * @param value
     *      String to convert.
     *
     * @return
     *      Converted string.
     */
    static std::string toLower(const std::string& value)
    {
        std::string data = value;
        for (auto& c : data)
        {
            c = toLower(c);
        }
        return data;
    }

    /**
     * Converts string to lower-case.
     *
     * @param value
     *      The string to search in.
     * @param ending
     *      The string to find.
     *
     * @return
     *      True if value ends with the ending, false otherwise.
     */
    static bool endWith(const std::string& value,
                        const std::string& ending)
    {
        if (value.length() < ending.length())
        {
            return false;
        }

        return (value.substr(value.length() - ending.length()) == ending);
    }

    /**
     * Trims the string (removes whitespaces) from begin.
     *
     * @param value
     *      Value to trim.
     *
     * @return
     *      Trimmed value.
     */
    static std::string trimBegin(const std::string& value);

    /**
     * Trims the string (removes whitespaces) from end.
     *
     * @param value
     *      Value to trim.
     *
     * @return
     *      Trimmed value.
     */
    static std::string trimEnd(const std::string& value);

    /**
     * Trims the string (removes whitespaces) from begin and end.
     *
     * @param value
     *      Value to trim.
     *
     * @return
     *      Trimmed value.
     */
    static std::string trim(const std::string& value);

    /**
     * Creates string using printf-like format.
     *
     * @param format
     *      Printf-like format string.
     *
     * @return
     *      Created string.
     */
    static std::string format(const char* format,
                              ...);

    /**
     * Creates string using printf-like format.
     *
     * @param format
     *      Printf-like format string.
     * @param arguments
     *      Arguments for printf.
     *
     * @return
     *      Created string.
     */
    static std::string vformat(const char* format,
                               std::va_list arguments);

private:
    /** Safe character value range for std:: character operations. */
    static const int SAFE_CHAR_MIN;
    /** Safe character value range for std:: character operations. */
    static const int SAFE_CHAR_MAX;

};

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_STRINGUTILS_HPP_*/
