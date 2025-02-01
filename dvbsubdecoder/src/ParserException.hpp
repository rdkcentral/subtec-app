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


#ifndef DVBSUBDECODER_PARSEREXCEPTION_HPP_
#define DVBSUBDECODER_PARSEREXCEPTION_HPP_

#include <exception>

namespace dvbsubdecoder
{

/**
 * Exception throw on parsing errors.
 */
class ParserException : public std::exception
{
public:
    /**
     * Constructor
     *
     * @param message
     *      Exception message.
     */
    ParserException(const char* message) :
            std::exception(),
            m_message(message)
    {
        // noop
    }

    /**
     * Returns exception message.
     *
     * Returns a C-style character string describing the general cause
     * of the current error.
     *
     * @return
     *      Message given to constructor.
     */
    virtual const char* what() const noexcept override
    {
        return m_message ? m_message : "";
    }

private:
    /** Exception message. */
    const char* const m_message;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_PARSEREXCEPTION_HPP_*/
