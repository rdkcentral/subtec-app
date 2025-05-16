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


#pragma once

#include <vector>
#include <memory>
#include <map>

#include "Elements.hpp"
#include "Timing.hpp"

#include <subttxrend/gfx/Types.hpp>

namespace subttxrend
{
namespace ttmlengine
{

/**
 * Helper function for checking shared_ptr referenced objects for equality.
 *
 * @param a
 *      First object pointer.
 * @param b
 *      Second object pointer.
 * @return
 *      True if both pointers points to equal (not necessarily the same) objects, false otherwise.
 */
template<class T, class U>
bool sharedPtrObjectsEqual(const std::shared_ptr<T>& a,
                           const std::shared_ptr<U>& b)
{
    if (a == b)
    {
        return true;
    }
    if (a && b)
    {
        return *a == *b;
    }
    return false;
}

/**
 * Represent content to be presented on screen for given period of time.
 * Content can be:
 * - an image
 * - number of line of text consisting of number of text chunks
 */
class IntermediateDocument
{
public:

    /**
     * Represents continuous part of text with the same style.
     */
    struct TextChunk
    {
        /**
         * Equality operator.
         *
         * @param lhs
         * @param rhs
         * @return
         *      True if contents of both TextChunks are the same. False otherwise.
         */
        friend bool operator==(const TextChunk& lhs,
                               const TextChunk& rhs)
        {
            return ((lhs.m_text == rhs.m_text) && (lhs.m_style == rhs.m_style)
                    && (lhs.m_forceNewline == rhs.m_forceNewline));
        }

        /** Text content. */
        std::string m_text;

        /** Text style attributes. */
        StyleSet m_style;

        /** Force newline flag. */
        bool m_forceNewline = false;

        XmlSpace m_whitespaceHandling = XmlSpace::DEFAULT;
    };

    struct ImageChunk
    {
        friend bool operator==(const ImageChunk& lhs,
                               const ImageChunk& rhs)
        {
            return sharedPtrObjectsEqual(lhs.m_image, rhs.m_image);
        }

        friend std::ostream& operator<<(std::ostream& stream, const ImageChunk& chunk)
        {
            if (chunk.m_image && chunk.m_image->getId() != "")
            {
                stream << "image: " << chunk.m_image->getId();
            }
            return stream;
        }

        std::shared_ptr<ImageElement> m_image;
        std::shared_ptr<gfx::Bitmap> m_bmp{};
    };

    using TextLine = std::vector<TextChunk>;

    /**
     * Represents contents of one region. Can contain few text chunks with different format.
     */
    struct Entity
    {
        /**
         * Equality operator.
         *
         * @param lhs
         * @param rhs
         * @return
         *      True if contents of both TextEntities are the same. False otherwise.
         */
        friend bool operator==(const Entity& lhs,
                               const Entity& rhs)
        {
            return ((lhs.m_textLines == rhs.m_textLines) && sharedPtrObjectsEqual(lhs.m_region, rhs.m_region)
                && (lhs.m_imageChunk == rhs.m_imageChunk));
        }

        friend std::ostream& operator<<(std::ostream& stream, const Entity& entity)
        {
            for (const auto& textLine : entity.m_textLines)
            {
                for(const auto& textChunk : textLine)
                {
                    if (not textChunk.m_text.empty()){
                        stream << " [" << textChunk.m_text << "]";
                    }
                }
                stream << "\n";
            }
            stream << entity.m_imageChunk;
            return stream;
        }

        bool isSameImage(Entity const& other) const
        {
            if (m_textLines.empty() && other.m_textLines.empty()) {
                return m_imageChunk.m_image && other.m_imageChunk.m_image &&
                       m_imageChunk.m_image->getId() == other.m_imageChunk.m_image->getId();
            }
            // always false for text ttml
            return false;
        }

        bool empty()
        {
            auto retVal{true};
            if ((m_imageChunk.m_image && m_imageChunk.m_image->getId() != "") ||
                (m_textLines.size() > 0))
            {
                retVal = false;
            }
            return retVal;
        }

        /** Text lines container. */
        std::vector<TextLine> m_textLines;

        /** Image content. */
        ImageChunk m_imageChunk;

        /** Region where content should be presented. */
        std::shared_ptr<RegionElement> m_region;
    };

    /**
     * Return string representation.
     *
     * @return
     *      Document string representation.
     */
    std::string toStr()
    {
        std::ostringstream str;
        str << "[" << m_timing.toStr() << "]: ";
        for(const auto& entity : m_entites)
        {
            str << entity ;
        }
        std::string rStr = str.str();
        rStr = std::regex_replace( rStr,
			           std::regex(R"(\s+)"),
				   " ");
        return rStr;
    }

    /**
     * 'Less' operator.
     *
     * @param lhs
     * @param rhs
     *
     * @return
     *      True if first object is earlier in time than the second.
     */
    friend bool operator<(const IntermediateDocument& lhs,
                          const IntermediateDocument& rhs)
    {
        return lhs.m_timing < rhs.m_timing;
    }

    /**
     * Equality operator.
     *
     * @param lhs
     * @param rhs
     * @return
     *      True if element are the same. False otherwise.
     */
    friend bool operator==(const IntermediateDocument& lhs,
                           const IntermediateDocument& rhs)
    {
        return (lhs.m_entites == rhs.m_entites) && (lhs.m_timing == rhs.m_timing);
    }

    /**
     * Apply display offset to the current display start / end.
     *
     * @param offsetMs display offset in ms that is going to be added
     */
    void applyDisplayOffset(std::int64_t offsetMs)
    {
        if (offsetMs != 0)
        {
            m_timing.applyOffset(offsetMs);
        }
    }

    bool isSameImage(const IntermediateDocument& other) const
    {
        if (m_entites.size() == other.m_entites.size()) {
            return std::equal(m_entites.cbegin(),
                              m_entites.cend(),
                              other.m_entites.cbegin(),
                              [](Entity const& lhs, Entity const& rhs) { return lhs.isSameImage(rhs); });
        }
        return false;
    }

    void setCellResolution(const gfx::Size& cellResolution)
    {
        m_cellResolution = cellResolution;
    }

    /** Text content. */
    std::vector<Entity> m_entites;

    /** Timing. */
    Timing m_timing;

    gfx::Size m_cellResolution;
};

} // namespace subttxrend
} // namespace ttmlengine

