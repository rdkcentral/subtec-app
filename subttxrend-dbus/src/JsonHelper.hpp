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


#ifndef _SUBTTXREND_DBUS_SRC_JSONHELPER_HPP_
#define _SUBTTXREND_DBUS_SRC_JSONHELPER_HPP_

#include <string>

#include "SubtitleStatus.hpp"
#include "TeletextStatus.hpp"

namespace subttxrend
{
namespace dbus
{

/**
 * Set of helper function for encoding and decoding json formated dbus messages.
 *
 */
class JsonHelper
{
public:
    JsonHelper() = delete;
    ~JsonHelper() = delete;

    /**
     * Decode subtitles setMuted request.
     *
     * @param jsonText
     *      String with Json encoded request.
     *
     * @return
     *      Value of parsed request.
     */
    static bool DecodeSubtitleSetMuted(const std::string &jsonText);

    /**
     * Decode subtitles getStatus request.
     *
     * @param status
     *      Status data to be encoded.
     *
     * @return
     *      String with Json encoded status.
     */
    static std::string EncodeSubtitleStatusResponse(const SubtitleStatus &status);

    /**
     * Decode teletext setMuted request.
     *
     * @param jsonText
     *      String with Json encoded request.
     *
     * @return
     *      Value of parsed request.
     */
    static bool DecodeTeletextSetMuted(const std::string &jsonText);

    /**
     * Decode teletext getStatus request.
     *
     * @param status
     *      Status data to be encoded.
     *
     * @return
     *      String with Json encoded status.
     */
    static std::string EncodeTeletextStatusResponse(const TeletextStatus &status);

private:

    /**
     * Decode setMuted request.
     *
     * setMuted is in the same format for subtitles and teletext so common implementation is used.
     *
     * @param jsonText
     *      String with json encoded request.
     *
     * @return
     *      Decoded setMuted value.
     */
    static bool DecodeSetMuted(const std::string &jsonText);

    /** logger instance */
    static common::Logger m_logger;

};

} // namespace dbus
} // namespace subttxrend

#endif /* _SUBTTXREND_DBUS_SRC_JSONHELPER_HPP_ */
