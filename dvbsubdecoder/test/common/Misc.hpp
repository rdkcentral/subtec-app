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


#ifndef DVBSUBDECODER_MISC_HPP_
#define DVBSUBDECODER_MISC_HPP_

#include "Types.hpp"

bool operator ==(const dvbsubdecoder::StcTime& time1,
                 const dvbsubdecoder::StcTime& time2)
{
    return (time1.m_type == time2.m_type) && (time1.m_time == time2.m_time);
}

bool operator ==(const dvbsubdecoder::Rectangle& rect1,
                 const dvbsubdecoder::Rectangle& rect2)
{
    return (rect1.m_x1 == rect2.m_x1) && (rect1.m_x2 == rect2.m_x2)
            && (rect1.m_y1 == rect2.m_y1) && (rect1.m_y2 == rect2.m_y2);
}

#endif /*DVBSUBDECODER_MISC_HPP_*/
