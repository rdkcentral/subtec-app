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

#include "AttributeHandlers.hpp"
#include "StyleSet.hpp"
#include "Utils.hpp"

#include <cassert>
#include <memory>
#include <regex>
#include <set>
#include <unordered_map>

#include <subttxrend/gfx/Types.hpp>

namespace subttxrend
{
namespace ttmlengine
{

struct TextLine
{
    // Marks the text line to be added by detecting new line mark (<br>) - true
    // false - otherwise
    bool isForcedLine{false};
    std::string text;
};

/**
 * Class representing generic ttml element.
 */
class Element
{
public:

    /**
     * Constructor.
     */
    Element() = default;

    /**
     * Destructor.
     */
    virtual ~Element() = 0;

    /**
     * Parse attribute.
     *
     * @param name
     *      Attribute name.
     * @param value
     *      Attribute value.
     */
    void parseAttribute(const std::string& prefix,
                        const std::string& name,
                        const std::string& value)
    {
        auto it = m_prefixHandlerMap.find(prefix);

        if (it != m_prefixHandlerMap.end())
        {
            it->second->addAttribute(name, value);
        }
        // TODO  move all attribute processing to handlers
        if (name == "id")
        {
            m_id = value;
        }
        else
        {
            parseAttributeDerived(name, value);
        }
    }

    /**
     * xml:id attribute getter.
     *
     * @return
     *      xml:id attribute value.
     */
    const std::string& getId() const
    {
        return m_id;
    }

    /**
     * Check if given element could have textual content.
     *
     * @return
     *      True if element is expected to contain text, false otherwise.
     */
    virtual bool expectTextContent() const
    {
        return false;
    }

    /**
     * Appends text content.
     *
     * @param
     *      Text content to add.
     */
    virtual void appendText(const std::string& text)
    {
        // make sure it's overridden for derived elements expecting text
        assert(expectTextContent());
    }

    /**
     * Appends new text line.
     */
    virtual void appendNewline()
    {
        // noop
    }

protected:

    /**
     * Parse any derived class specific attribute.
     *
     * @param name
     *      Attribute name.
     * @param value
     *      Attribute value.
     */
    virtual void parseAttributeDerived(const std::string& name,
                                       const std::string& value)
    {
        // noop
    }

    void registerAttributeHandler(const std::string& prefix, AttributeHandler* handler)
    {
        assert(handler);
        assert(m_prefixHandlerMap.find(prefix) == m_prefixHandlerMap.end());
        m_prefixHandlerMap.insert({prefix, handler});
    }

private:

    /** xml:id of this element. */
    std::string m_id;

    using AttributeHandlersMap = std::unordered_map<std::string, AttributeHandler*>;
    AttributeHandlersMap m_prefixHandlerMap;
};

inline Element::~Element()
{
    // noop
}

/**
 * Class representing ttml style element.
 */
class StyleElement : public Element,
                     public StyleAttributeHandler
{
public:

    StyleElement()
    {
        registerAttributeHandler("tts", this);
    }
};

/**
 * Class representing ttml region data.
 */
class RegionElement : public Element,
                      public StyleAttributeHandler
{
public:

    /**
     * Constructor.
     *
     * @param x
     *      X coordinate of the region in  percentage times 100.
     * @param y
     *      Y coordinate of the region in  percentage times 100.
     * @param width
     *      Width of the region in  percentage times 100.
     * @param height
     *      Height of the region in  percentage times 100.
     */
    RegionElement(int x,
                  int y,
                  int width,
                  int height)
            : m_x(DomainValue::Type::PERCENTAGE_HUNDREDTHS, x),
              m_y(DomainValue::Type::PERCENTAGE_HUNDREDTHS, y),
              m_width(DomainValue::Type::PERCENTAGE_HUNDREDTHS, width),
              m_height(DomainValue::Type::PERCENTAGE_HUNDREDTHS, height)
    {
        registerAttributeHandler("tts", this);
    }

    // default region should be equal root container region
    RegionElement() : RegionElement(0, 0, 10000, 10000)
    {
    }

    void addAttribute(const std::string& name, const std::string& value) override
    {
        if (name == "extent") {
            parsePoint(value, m_width, m_height);
        } else if (name == "origin") {
            parsePoint(value, m_x, m_y);
        } else {
            StyleAttributeHandler::addAttribute(name, value);
        }
    }

    /**
     * Get x coordinate in percentage times 100 of screen width.
     *
     * @return
     *      Region starting x coordinate in percentage of screen width.
     */
    DomainValue getX() const
    {
        return m_x;
    }

    /**
     * Get y coordinate in percentage times 100 of screen height.
     *
     * @return
     *      Region starting y coordinate in percentage of screen height.
     */
    DomainValue getY() const
    {
        return m_y;
    }

    /**
     * Get width in percentage times 100 of screen width.
     *
     * @return
     *      Region width in percentage of screen width.
     */
    DomainValue getWidth() const
    {
        return m_width;
    }

    /**
     * Get height in percentage times 100 of screen height.
     *
     * @return
     *      Region height in percentage of screen height.
     */
    DomainValue getHeight() const
    {
        return m_height;
    }

    /**
     * Style id getter.
     *
     * @return
     *      Content style id.
     */
    std::string getStyleId()
    {
        return m_styleRef;
    }

    /**
     * Equality operator.
     *
     * @param lhs
     * @param rhs
     * @return
     *      True if object represent the same region. False otherwise.
     */
    friend bool operator==(const RegionElement& lhs,
                           const RegionElement& rhs)
    {
        return ((lhs.getId() == rhs.getId()) && (lhs.m_height == rhs.m_height) && (lhs.m_width == rhs.m_width)
                && (lhs.m_x == rhs.m_x) && (lhs.m_y == rhs.m_y));
    }

    std::string toStr()
    {
        std::ostringstream str;
        str << "[" << getId() << "]: [" << m_x << ", " << m_y << ", " << m_width << ", " << m_height <<']';
        return str.str();
    }

protected:
    /** @copydoc Element::parseAttributeDerived */
    virtual void parseAttributeDerived(const std::string& name,
                                       const std::string& value) override
    {
        if (name == "style")
        {
            m_styleRef = value;
        }
    }

private:

    /**
     * Parses given text for percentage values.
     *
     * @param text
     *      Text to parse.
     * @param value1
     *      Reference for returning first value found. Filled with 0 on error.
     * @param value2
     *      Reference for returning second value found. Filled with  0 on error.
     */
    void parsePoint(const std::string& text,
                    DomainValue& value1,
                    DomainValue& value2)
    {
        value1 = {DomainValue::Type::PIXEL, 0};
        value2 = {DomainValue::Type::PIXEL, 0};

        // percent dimensions regex pattern - for example: 2.84% 85.23%
        static const std::regex pointPercentPattern("([0-9]+\\.?[0-9]*)% ([0-9]+\\.?[0-9]*)%");
        static const std::regex pointPixelPattern("([0-9]+)px ([0-9]+)px");

        std::smatch match;
        if (std::regex_match(text, match, pointPercentPattern))
        {
            try
            {
                float value1Float = std::stof(match[1].str());
                float value2Float = std::stof(match[2].str());

                value1 = {DomainValue::Type::PERCENTAGE_HUNDREDTHS, static_cast<int>(value1Float * 100)};
                value2 = {DomainValue::Type::PERCENTAGE_HUNDREDTHS, static_cast<int>(value2Float * 100)};
            }
            catch (...)
            {
                // noop
            }
        }
        else if (std::regex_match(text, match, pointPixelPattern))
        {
            try
            {
                value1 = {DomainValue::Type::PIXEL, std::stoi(match[1].str())};
                value2 = {DomainValue::Type::PIXEL, std::stoi(match[2].str())};
            }
            catch (...)
            {
                value1 = {DomainValue::Type::PIXEL, 0};
                value2 = {DomainValue::Type::PIXEL, 0};
            }
        }
    }

private:
    DomainValue m_x;
    DomainValue m_y;
    DomainValue m_width;
    DomainValue m_height;

    std::string m_styleRef;
};

/**
 * Class representing ttml image data.
 */
class ImageElement : public Element
{
public:

    /**
     * Constructor.
     */
    ImageElement() :
            m_base64ImageData(std::make_shared<std::string>())
    {
        // noop
    }

    /** @copydoc Element::expectTextContent */
    virtual bool expectTextContent() const override
    {
        return true;
    }

    /** @copydoc Element::appendText */
    virtual void appendText(const std::string& text) override
    {
        m_base64ImageData->append(text);
    }

    /**
     * Image data getter.
     *
     * @return
     *      Base64 encoded string containing image data.
     */
    const std::shared_ptr<std::string> getBase64Data()
    {
        return m_base64ImageData;
    }

    bool isSameImage(std::shared_ptr<ImageElement> const& other) const
    {
        auto isSame{false};
        if (other && (getId() == other->getId()) && (m_base64ImageData->size() == other->m_base64ImageData->size())) {
            isSame = !m_base64ImageData->compare(*(other->m_base64ImageData));
        }
        return isSame;
    }

    /**
     * Equality operator.
     *
     * @param lhs
     * @param rhs
     * @return
     *      True if object represent the same region. False otherwise.
     */
    friend bool operator==(const ImageElement& lhs,
                           const ImageElement& rhs)
    {
        return ((lhs.getId() == rhs.getId()) && (lhs.m_base64ImageData == rhs.m_base64ImageData));
    }

protected:

    /** @copydoc Element::parseAttributeDerived */
    virtual void parseAttributeDerived(const std::string& name,
                                       const std::string& value) override
    {
        // TODO: imagetype, and encoding
    }

private:

    /** Image data as presented in document - base64 encoded. */
    std::shared_ptr<std::string> m_base64ImageData;
};

/**
 * Element with text content.
 */
class BodyElement : public Element,
                    public StyleAttributeHandler
{
public:

    /**
     * Constructor.
     */
    BodyElement() :
            m_whitespaceHandling(XmlSpace::DEFAULT),
            m_styleRef(),
            m_regionRef(),
            m_timing(),
            m_textLines({{}})
    {
        registerAttributeHandler("tts", this);
    }

    /**
     * Constructor. Text attributes are inherited from given object.
     *
     * @param parent
     *      Object to inherit properties from.
     */
    BodyElement(const std::shared_ptr<BodyElement>& parent) :
            m_parent(parent),
            m_whitespaceHandling(parent->m_whitespaceHandling),
            m_styleRef(parent->m_styleRef),
            m_regionRef(parent->m_regionRef),
            m_timing(parent->m_timing),
            m_textLines({{}})
    {
        registerAttributeHandler("tts", this);
    }

    /**
     * Destructor.
     */
    virtual ~BodyElement()
    {
        // noop
    }

    /** @copydoc Element::expectTextContent */
    virtual bool expectTextContent() const override
    {
        return true;
    }

    /** @copydoc Element::appendText */
    virtual void appendText(const std::string& text) override
    {
        if (not text.empty())
        {
            std::string& currentLine = m_textLines.back().text;

            currentLine.append(text);
        }
    }

    virtual void appendNewline() override
    {
        m_textLines.push_back({true, {}});
    }

    /**
     * Style id getter.
     *
     * @return
     *      Content style id.
     */
    std::string getStyleId()
    {
        return m_styleRef;
    }

    /**
     * Region id getter.
     *
     * @return
     *      Content region id.
     */
    std::string getRegionId()
    {
        return m_regionRef;
    }

    /**
     * Timing getter.
     *
     * @return
     *      Object timing data.
     */
    Timing getTiming()
    {
        return m_timing;
    }

    /**
     * Text lines getter.
     *
     * @return
     *      Text line vector.
     */
    const std::vector<TextLine>& getTextLines()
    {
        return m_textLines;
    }

    /**
     * Background image id getter.
     *
     * @return
     *      Background image id.
     */
    std::string getBackgroundImageId()
    {
        return m_backgroundImageId;
    }

    /**
     * Pointer to parent getter.
     *
     * @return
     *      Pointer to parent.
     */
    std::shared_ptr<BodyElement> getParent()
    {
        return m_parent;
    }

    /**
     * White space handling getter.
     *
     * @return
     *      white space handling.
     */
    XmlSpace getWhiteSpaceHandling()
    {
        return m_whitespaceHandling;
    }

    bool hasValidContent()
    {
        return (not m_textLines.empty() or not m_backgroundImageId.empty());
    }

    void finalize()
    {
        for (auto it = m_textLines.begin(); it != m_textLines.end(); )
        {
            auto & linePair = *it;
            if (m_whitespaceHandling == XmlSpace::DEFAULT)
            {
                linePair.text = trimWhitespace(linePair.text);
            }
            if (linePair.isForcedLine == false && linePair.text.empty())
            {
                it = m_textLines.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

protected:

    /** @copydoc Element::parseAttributeDerived */
    virtual void parseAttributeDerived(const std::string& name,
                                       const std::string& value) override
    {
        if (name == "begin")
        {
            parseTimepoint(value, m_timing.getStartTimeRef());
        }
        else if (name == "end")
        {
            parseTimepoint(value, m_timing.getEndTimeRef());
        }
        else if (name == "space")
        {
            if (value == "default")
            {
                m_whitespaceHandling = XmlSpace::DEFAULT;
            }
            else if (value == "preserve")
            {
                m_whitespaceHandling = XmlSpace::PRESERVE;
            }
        }
        else if (name == "style")
        {
            m_styleRef = value;
        }
        else if (name == "region")
        {
            m_regionRef = value;
        }
        else if (name == "backgroundImage")
        {
            if ((value.length() > 1) && (value[0] == '#'))
            {
                m_backgroundImageId = value.substr(1);
            }
        }
    }

private:

    /**
     * Parses timepoint string in format 'hours:minutes:seconds[.fraction]'.
     *
     * @param value
     *      Text containing timepoint.
     * @param timePointRef
     *      Reference to timepoint object to place value.
     */
    void parseTimepoint(const std::string& value,
                        TimePoint& timePointRef)
    {
        // media time regex pattern - hours:minutes:seconds[.fraction]
        static const std::regex timingPattern("([0-9]+):([0-9][0-9]?):([0-9][0-9]?)\\.?([0-9]+)?");

        std::smatch match;

        if (std::regex_match(value, match, timingPattern))
        {
            try
            {
                using namespace std::chrono;

                hours hoursNum{std::stoi(match[1].str())};
                minutes minutesNum{std::stoi(match[2].str())};
                seconds secondsNum{std::stoi(match[3].str())};
                milliseconds millisNum{0};

                // fraction part is optional
                if ((match[4].matched))
                {
                    millisNum = milliseconds{std::stoi(match[4].str())};
                }
                timePointRef = TimePoint(hoursNum, minutesNum, secondsNum, millisNum);
            }
            catch (...)
            {
                // TODO trace/debug msg
                // reset to defaults
                timePointRef = TimePoint();
            }
        }
    }

    /** Parent element */
    std::shared_ptr<BodyElement> m_parent;

    /** How whitespace should be processed. */
    XmlSpace m_whitespaceHandling;

    /** Style Id. */
    std::string m_styleRef;

    /** Region Id. */
    std::string m_regionRef;

    /** Active time period. */
    Timing m_timing;

    /** Text content. */
    std::vector<TextLine> m_textLines;

    /** Background image id */
    std::string m_backgroundImageId;
};

// Ttml specification defines default number of columns and rows to be 32 and 15, respectively.
const gfx::Size DEFAULT_CELL_RESOLUTION{32, 15};

/**
 * TT - root document element.
 */
class TTElement : public Element
{
public:

    TTElement() :
        m_cellResolution{DEFAULT_CELL_RESOLUTION}
    {
    }

    virtual ~TTElement()
    {
        // noop
    }

    const gfx::Size& getCellResolution()
    {
        return m_cellResolution;
    }

protected:

    /** @copydoc Element::parseAttributeDerived */
    virtual void parseAttributeDerived(const std::string& name,
                                       const std::string& value) override
    {
        if (name == "cellResolution")
        {
            parseResolution(value, m_cellResolution.m_w, m_cellResolution.m_h);
            if (m_cellResolution.m_w == 0 || m_cellResolution.m_h == 0)
            {
                m_cellResolution = DEFAULT_CELL_RESOLUTION;
            }
        }
    }

private:

    /**
     * Parses given text for integer values.
     *
     * @param text
     *      Text to parse.
     * @param value1
     *      Reference for returning first value found. Filled with 0 on error.
     * @param value2
     *      Reference for returning second value found. Filled with  0 on error.
     */
    void parseResolution(const std::string& text, int& width, int& height)
    {
        width = 0;
        height = 0;

        // resolution regex pattern - for example: "41 24"
        static const std::regex resolutionPattern(" *([0-9]+) +([0-9]+) *");

        std::smatch match;
        if (std::regex_match(text, match, resolutionPattern))
        {
            try
            {
                width = std::stoi(match[1].str());
                height = std::stoi(match[2].str());
            }
            catch (...)
            {
                width = 0;
                height = 0;
            }
        }
    }

private:
    gfx::Size m_cellResolution;
};

} // namespace subttxrend
} // namespace ttmlengine

