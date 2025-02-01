/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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

#include <cstdint>
#include <map>
#include <mutex>

namespace subttxrend {
namespace webvttengine {

/**
 * @brief WebVTT attributea class.
 */
class WebVTTAttributes
{
public:
    /**
     * @brief Attribute types.
     */
    enum class AttributeType
    {
        UNDEF = 0,
        FONT_COLOR,
        BACKGROUND_COLOR,
        FONT_OPACITY,
        BACKGROUND_OPACITY,
        FONT_STYLE,
        FONT_SIZE,
        WINDOW_COLOR,
        WINDOW_OPACITY,
        EDGE_STYLE,
        EDGE_COLOR,
        LAST_VALUE
    };

    /**
     * @brief Font sizes.
     */
    enum class FontSize
    {
        SMALL = 0,
        MEDIUM,
        LARGE,
        EXTRA_LARGE
    };

    enum class FontStyle
    {
        DEFAULT_OR_UNDEFINED = 0,
        MONOSPACED_WITH_SERIFS,
        PROPORTIONALLY_SPACED_WITH_SERIFS,
        MONOSPACED_WITHOUT_SERIFS,
        PROPORTIONALLY_SPACED_WITHOUT_SERIFS,
        CASUAL_FONT_TYPE,
        CURSIVE_FONT_TYPE,
        SMALL_CAPITALS,
        DEFAULT = DEFAULT_OR_UNDEFINED
    };

    enum class EdgeStyle
    {
        NONE = 0,
        RAISED,
        DEPRESSED,
        UNIFORM,
        LEFT_DROP_SHADOW,
        RIGHT_DROP_SHADOW,
        DEFAULT = NONE
    };

    enum class Opacity
    {
        SOLID = 0,
        FLASHING,
        TRANSLUCENT,
        TRANSPARENT
    };

    /**
     * @brief Default constructor.
     *
     * @param attributeType
     *      Attribute type.
     */
    WebVTTAttributes()
    {
    }

    /**
     * @brief Copy constructor.
     *
     * @param attributes
     *      Initial attributes.
     */
    WebVTTAttributes(const WebVTTAttributes &attributes)
    {
        std::lock_guard<std::mutex> lock(attributes.m_mutex);
        m_attributes = attributes.m_attributes;
    }

    /**
     * @brief Assignment operator.
     *
     * @param attributes
     *      Initial attributes.
     */
    WebVTTAttributes operator=(const WebVTTAttributes &attributes)
    {
        return WebVTTAttributes(attributes);
    }

    /**
     * @brief Is attribute set.
     *
     * @param attributeType
     *      Attribute type.
     * @return
     *      true if the attribute is set, false otherwise.
     */
    bool isSet(const AttributeType attributeType) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto attribute = m_attributes.find(attributeType);

        return ((attribute != m_attributes.end()) ? true : false);
    }

    /**
     * @brief Get an integer attribute value.
     *
     * @param attributeType
     *      Attribute type.
     * @return
     *      Attribute value if set.
     */
    uint32_t getInteger(const AttributeType attributeType) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        uint32_t value = 0;
        auto attribute = m_attributes.find(attributeType);

        if ((attributeType < AttributeType::LAST_VALUE) &&
            (attribute != m_attributes.end()))
        {
            value = attribute->second;
        }

        return value;
    }

    /**
     * @brief Set an integer attribute value.
     *
     * @param attributeType
     *      Attribute type.
     * @param value
     *      Attribute value.
     */
    void setInteger(const AttributeType attributeType, uint32_t value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (attributeType < AttributeType::LAST_VALUE)
        {
            m_attributes[attributeType] = value;
        }
    }

    /**
     * @brief Update attributes.
     *
     * @param attributes
     *      New attributes.
     */
    void update(const WebVTTAttributes &attributes)
    {
        std::lock_guard<std::mutex> lock1(attributes.m_mutex);
        for (auto &attribute : attributes.m_attributes)
        {
            std::lock_guard<std::mutex> lock2(m_mutex);
            m_attributes[attribute.first] = attribute.second;
        }
    }

    /**
     * @brief Reset attributes.
     */
    void reset()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_attributes.clear();
    }

private:
    /** @brief Attributes. */
    std::map<AttributeType, uint32_t> m_attributes;

    /** @brief Mutex. */
    mutable std::mutex m_mutex;
};

}
}
