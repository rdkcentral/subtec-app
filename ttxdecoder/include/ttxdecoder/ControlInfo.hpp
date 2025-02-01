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


#ifndef TTXDECODER_CONTROLINFO_HPP_
#define TTXDECODER_CONTROLINFO_HPP_

#include <cstdint>

namespace ttxdecoder
{

/**
 * Page control info bits.
 */
class ControlInfo
{
    ControlInfo() = delete;

public:
    /** C4 - Erase Page. */
    static const std::uint8_t ERASE_PAGE = (1 << 0);

    /** C5 - Newsflash. */
    static const std::uint8_t NEWSFLASH = (1 << 1);

    /** C6 - Subtitle. */
    static const std::uint8_t SUBTITLE = (1 << 2);

    /** C7 - Suppress Header. */
    static const std::uint8_t SUPRESS_HEADER = (1 << 3);

    /** C8 - Update Indicator. */
    static const std::uint8_t UPDATE_INDICATOR = (1 << 4);

    /** C9 - Interrupted Sequence. */
    static const std::uint8_t INTERRUPTED_SEQUENCE = (1 << 5);

    /** C10 - Inhibit Display. */
    static const std::uint8_t INHIBIT_DISPLAY = (1 << 6);

    /** C11 - Magazine Serial. */
    static const std::uint8_t MAGAZINE_SERIAL = (1 << 7);

};

} // namespace ttxdecoder

#endif /*TTXDECODER_CONTROLINFO_HPP_*/
