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

#include <exception>
#include <string>

#ifdef __APPLE__
#undef _GLIBCXX_USE_NOEXCEPT
#define _GLIBCXX_USE_NOEXCEPT _NOEXCEPT
#endif


class WebVTTException : public std::exception
{

public:
    /** Takes a character string describing the error.  */
    explicit WebVTTException(const std::string& __arg) : std::exception(), _M_msg(__arg) {}

    /** Returns a C-style character string describing the general cause of
     *  the current error (the same string passed to the ctor).  */
    const char* what() const _GLIBCXX_USE_NOEXCEPT override
    {
        return _M_msg.c_str();
    }

protected:
    std::string name;
    
private:
    std::string _M_msg;
};

class ParserException : public WebVTTException
{
    using WebVTTException::WebVTTException;
};

class EndOfFileException : public WebVTTException
{
    using WebVTTException::WebVTTException;
};

class InvalidCueException : public WebVTTException
{
    using WebVTTException::WebVTTException;
};

class DisplayException : public WebVTTException
{
    using WebVTTException::WebVTTException;
};
