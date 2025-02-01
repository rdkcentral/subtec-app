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


#ifndef FTCPP_EXCEPTION_HPP_
#define FTCPP_EXCEPTION_HPP_

#include <stdexcept>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <sstream>

namespace ftcpp
{

/**
 * Freetype exception thrown on errors.
 */
class Exception : public std::exception
{
public:
    /**
     * Constructor.
     *
     * @param error
     *      Freetype error code.
     */

    const char* getErrorString(FT_Error err)
    {
        #undef FTERRORS_H_
        #define FT_ERRORDEF( e, v, s )  case e: return s;
        #define FT_ERROR_START_LIST     switch (err) {
        #define FT_ERROR_END_LIST       }
        #include FT_ERRORS_H
        return "Unknown error";
    }

    Exception(const std::string& str, FT_Error error)
    {
        std::stringstream message;

        message << str << " FreeType error code: " << error << " \"" << getErrorString(error) << "\"" << std::endl;

        m_message = message.str();
    }

    /** Returns a C-style character string describing the general cause
     *  of the current error.  */
    virtual const char* what() const noexcept override
    {
        return m_message.c_str();
    }

private:
    /** Exception message. */
    std::string m_message;
};

} // namespace ftcpp

#endif /*FTCPP_EXCEPTION_HPP_*/
