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


#ifndef TTXDECODER_PRESENTATIONLEVEL_HPP_
#define TTXDECODER_PRESENTATIONLEVEL_HPP_

namespace ttxdecoder
{

/** Presentation Level. */
enum class PresentationLevel
{
    /**
     * Presentation Level 1.
     */
    LEVEL_1,

    /**
     * Presentation Level 1.5.
     */
    LEVEL_1_5,

    /**
     * Presentation Level 2.5.
     *
     * @warning Not supported yet.
     */
    LEVEL_2_5,

    /**
     * Presentation Level 3.5.
     *
     * @warning Not supported yet.
     */
    LEVEL_3_5
};

} // namespace ttxdecoder

#endif /*TTXDECODER_PRESENTATIONLEVEL_HPP_*/
