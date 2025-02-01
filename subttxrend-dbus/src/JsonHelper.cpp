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


#include <memory>

#include <jansson.h>

#include <subttxrend/common/Logger.hpp>

#include "JsonHelper.hpp"

namespace subttxrend
{
namespace dbus
{

common::Logger JsonHelper::m_logger("Dbus", "JsonHelper");

/**
 * Helper type to allow automatic resource release.
 */
struct JsonObjDeleter
{
    /**
     * Releases the object.
     *
     * @param _ptr
     *      Object to be released.
     */
    void operator()(json_t* _ptr)
    {
        json_decref(_ptr);
    }
};
using unique_json_t = std::unique_ptr< json_t, JsonObjDeleter >;

/**
 * Helper type to allow automatic resource release.
 */
struct JsonDumpDeleter
{
    /**
     * Releases the object.
     *
     * @param _ptr
     *      Object to be released.
     */
    void operator()(char* _ptr)
    {
        free(_ptr);
    }
};
using unique_dump_t = std::unique_ptr< char, JsonDumpDeleter >;

bool JsonHelper::DecodeSetMuted(const std::string &jsonText)
{
    json_error_t error = json_error_t();
    unique_json_t root(json_loads(jsonText.c_str(), 0, &error));

    if (root == nullptr)
    {
        m_logger.warning("%s could not load json data (\'%s\')! error=%s", __func__, jsonText.c_str(), error.text);
        return false;
    }
    else
    {
        static const char *JANSSON_SET_MUTED_FMT = "{s:b}";
        constexpr int JANSSON_UNPACK_SUCCESS = 0;

        bool retValue = false;

        int unpackResult = json_unpack(root.get(), JANSSON_SET_MUTED_FMT, "muted", &retValue);

        if (unpackResult != JANSSON_UNPACK_SUCCESS)
        {
            m_logger.warning("%s error parsing setMuted request! jsonText=%s", __func__, jsonText.c_str());
        }

        m_logger.trace("%s parsed json data (\'%s\') - muted=%s", __func__, jsonText.c_str(),
                retValue ? "true" : "false");

        return retValue;
    }
}

bool JsonHelper::DecodeSubtitleSetMuted(const std::string &jsonText)
{
    // currently both subtitles and teletext works in the same way so
    // use the same implementation
    return DecodeSetMuted(jsonText);
}

std::string JsonHelper::EncodeSubtitleStatusResponse(const SubtitleStatus &status)
{
    static const char *JANSSON_SUBTITLE_RESPONSE_FMT = "{s:b, s:b, s:i, s:i s:i}";

    unique_json_t obj(
            json_pack(JANSSON_SUBTITLE_RESPONSE_FMT, "started", status.m_started, "muted", status.m_muted, "type",
                    status.m_type, "auxId1", status.m_auxId1, "auxId2", status.m_auxId2));

    if (obj == nullptr)
    {
        m_logger.error("%s could not encode status response", __func__);
        return std::string();
    }

    unique_dump_t response(json_dumps(obj.get(), JSON_PRESERVE_ORDER));
    if (response == nullptr)
    {
        m_logger.error("%s could not create json dump %p", __func__, obj.get());
        return std::string();
    }

    m_logger.trace("%s encoded status: \'%s\'", __func__, response.get());

    return response.get();
}

bool JsonHelper::DecodeTeletextSetMuted(const std::string &jsonText)
{
    // currently both subtitles and teletext works in the same way so
    // use the same implementation
    return DecodeSetMuted(jsonText);
}

std::string JsonHelper::EncodeTeletextStatusResponse(const TeletextStatus &status)
{
    static const char *JANSSON_TELETEXT_RESPONSE_FMT = "{s:b, s:b}";

    unique_json_t obj(json_pack(JANSSON_TELETEXT_RESPONSE_FMT, "started", status.m_started, "muted", status.m_muted));

    if (obj == nullptr)
    {
        m_logger.error("%s could not encode status response", __func__);
        return "";
    }

    unique_dump_t response(json_dumps(obj.get(), JSON_PRESERVE_ORDER));
    if (response == nullptr)
    {
        m_logger.error("%s could not create json dump %p", __func__, obj.get());
        return std::string();
    }

    m_logger.trace("%s encoded status: \'%s\'", __func__, response.get());

    return response.get();
}

} // namespace dbus
} // namespace subttxrend

