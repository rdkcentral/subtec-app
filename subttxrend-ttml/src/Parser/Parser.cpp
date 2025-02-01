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


#include <cassert>
#include <iomanip>

#include <cstdarg>

#include "Elements.hpp"
#include "Parser.hpp"

#include "XmlLibSaxParserWrapper.hpp"

namespace subttxrend
{
namespace ttmlengine
{

std::string regionToStr(const RegionElement& region)
{
    std::ostringstream oss;
    oss << region.getId() << " {" << region.getX() << ", " << region.getY() << ", "
            << region.getWidth() << ", " << region.getHeight() << "}";

    return oss.str();
}

void Parser::dumpTimeline(const std::list<IntermediateDocument>& timeline)
{
    if (m_logger.isEnabled(subttxrend::common::LoggerLevel::TRACE)) {
        m_logger.ostrace(__LOGGER_FUNC__, " TIMELINE(size=", timeline.size(), "):");
        for (auto &elem : timeline)
        {
            std::string str;

            str.append(elem.m_timing.toStr());

            for (const auto &entity : elem.m_entites)
            {
                str.append(" ");
                if (entity.m_region)
                {
                    str.append(regionToStr(*entity.m_region.get()));
                }
                if (entity.m_imageChunk.m_image)
                {
                    str.append(" imageId: [").append(entity.m_imageChunk.m_image->getId()).append("]");
                }
                for (const auto &textLine : entity.m_textLines)
                {
                    for (const auto &textChunk : textLine)
                    {
                        str.append(" ");
                        // TODO StyleSet operator <<
                        str.append(" ");
                        str.append(textChunk.m_text);
                    }
                }
            }
            m_logger.ostrace("\t ", str);
        }
    }
}

void Parser::shrinkTimelineTo(std::list<IntermediateDocument>& timeline, std::size_t requestedSize)
{
    assert(requestedSize <= MAX_TIMELINE_SIZE);

    while(timeline.size() > requestedSize)
    {
        m_logger.oswarning(__LOGGER_FUNC__, " dropping document because queue is full (size=", timeline.size(), ")");
        timeline.pop_front();
    }
}

void Parser::expandTimeline(std::list<IntermediateDocument> newTimeline)
{
    m_logger.ostrace(__LOGGER_FUNC__, " added ", newTimeline.size(), " to the timeline");

    newTimeline.sort();
    dumpTimeline(newTimeline);

    m_timeline = std::move(newTimeline);
}

std::list<IntermediateDocument> Parser::parse(const std::uint8_t* buffer, std::size_t bufferSize)
{
    m_logger.ostrace(__LOGGER_FUNC__, " parsing buffer: ", static_cast<const void*>(buffer), " size: ", bufferSize);

    m_saxParser.parse(buffer, bufferSize);
    std::list<IntermediateDocument> list;
    list.swap(m_timeline);

    // restart sax parser; parsed data was collected in callback functions
    m_saxParser.reset();
    return list;
}

void Parser::onStartDocument()
{
    m_logger.ostrace(__LOGGER_FUNC__);
}

void Parser::onEndDocument()
{
    m_logger.ostrace(__LOGGER_FUNC__);
    m_docInstance.dump();


    expandTimeline(m_docInstance.generateTimeline());

    m_docInstance.reset();
}

void Parser::onStartElementNs(const std::string& localname,
                              const std::string& prefix,
                              const std::string& URI,
                              const std::vector<Attribute>& attributes)
{
    m_logger.ostrace(__LOGGER_FUNC__, ' ', localname, ' ', prefix, ' ', URI);

    auto element = m_docInstance.startElement(localname);
    if (element)
    {
        parseAttributes(element.get(), attributes);
    }

    m_logger.ostrace(__LOGGER_FUNC__, ' ', localname);
}

void Parser::onEndElementNs(const std::string& localname,
                            const std::string& prefix,
                            const std::string& URI)
{
    m_logger.ostrace(__LOGGER_FUNC__,  ' ', localname, ' ', prefix, ' ', URI);

    m_docInstance.endElement();
}

void Parser::onCharacters(const std::string& content)
{
    m_logger.ostrace(__LOGGER_FUNC__, " content: ", content);

    auto element = m_docInstance.getCurrentElement();

    if (!element)
    {
        element = m_docInstance.getCurrentImageElement();
    }

    m_logger.ostrace(__LOGGER_FUNC__, " element=", element.get());

    if (element && element->expectTextContent())
    {
        element->appendText(content);
    }
}

void Parser::onError(const char * msg, ...)
{
}

void Parser::onWarning(const char * msg, ...)
{
}

void Parser::parseAttributes(Element* element,
                             const std::vector<Attribute>& attributes)
{
    assert(element);

    for (auto const& attr : attributes)
    {
        element->parseAttribute(attr.prefix, attr.name, attr.value);
        m_logger.ostrace(__LOGGER_FUNC__, " adding ", attr.prefix, ":", attr.name, "=", attr.value);
    }
}

} // namespace subttxrend
} // namespace ttmlengine
