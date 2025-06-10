/*****************************************************************************
* Copyright © 2018 Liberty Global B.V. and its Licensors.
* All rights reserved.
* Licensed by RDK Management, LLC under the terms of the RDK license.
* ============================================================================
* Liberty Global B.V. CONFIDENTIAL AND PROPRIETARY
* ============================================================================
* This file (and its contents) are the intellectual property of Liberty Global B.V.
* It may not be used, copied, distributed or otherwise disclosed in whole or in
* part without the express written permission of Liberty Global B.V.
* The RDK License agreement constitutes express written consent by Liberty Global.
* ============================================================================
* This software is the confidential and proprietary information of Liberty Global B.V.
* ("Confidential Information"). You shall not disclose this source code or
* such Confidential Information and shall use it only in accordance with the
* terms of the license agreement you entered into.
*
* LIBERTY GLOBAL B.V. MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
* SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE, OR NON-INFRINGEMENT. LIBERTY GLOBAL B.V. SHALL NOT BE LIABLE FOR
* ANY DAMAGES SUFFERED BY LICENSEE NOR SHALL THEY BE RESPONSIBLE AS A RESULT
* OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
******************************************************************************/

#include "DocumentInstance.hpp"

namespace subttxrend
{
namespace ttmlengine
{
DocumentInstance::DocumentInstance() :
        m_styles(),
        m_regions(),
        m_parsedBodyElementsStack(),
        m_content(),
        m_logger("TtmlEngine", "DocumentInstance")
{
    // noop
}

void DocumentInstance::reset()
{
    m_root.reset();
    m_regions.clear();
    m_styles.clear();
    m_content.clear();
    m_images.clear();

    while (!m_parsedBodyElementsStack.empty())
    {
        m_parsedBodyElementsStack.pop();
    }
    // We do not clear the override style attributes

    m_currentImageElement = std::shared_ptr<ImageElement>();
}

std::shared_ptr<Element> DocumentInstance::startElement(const std::string& name)
{
    m_logger.ostrace(__LOGGER_FUNC__, " name ", name);

    if (name == "image")
    {
        std::shared_ptr<ImageElement> img = std::make_shared<ImageElement>();
        m_images.insert(img);
        m_currentImageElement = img;
        return img;
    }
    else if (name == "region")
    {
        auto regionElement = std::make_shared<RegionElement>();
        m_regions.insert(regionElement);
        return regionElement;
    }
    else if (name == "style")
    {
        auto styleElement = std::make_shared<StyleElement>();
        m_styles.insert(styleElement);
        return styleElement;
    }
    else if (name == "body" || name == "div" || name == "p" || name == "span")
    {
        std::shared_ptr<BodyElement> bodyElement;
        if (m_parsedBodyElementsStack.empty())
        {
            bodyElement = std::make_shared<BodyElement>();
        }
        else
        {
            bodyElement = std::make_shared<BodyElement>(m_parsedBodyElementsStack.top());
        }
        m_parsedBodyElementsStack.push(bodyElement);
        m_content.push_back(bodyElement);
        return bodyElement;
    }
    else if (name == "tt")
    {
        if (m_root == nullptr)
        {
            m_root = std::make_shared<TTElement>();
        }
        else
        {
            m_logger.oswarning(__LOGGER_FUNC__, " more than one tt element in document");
        }
        return m_root;
    }
    else if (name == "br")
    {
        if (m_parsedBodyElementsStack.empty() == false)
        {
            auto& currentBodyElement = m_parsedBodyElementsStack.top();
            currentBodyElement->appendNewline();
        }
        m_parsedBodyElementsStack.push(nullptr);
        return nullptr;
    }
    else
    {
        return nullptr;
    }
}

void DocumentInstance::endElement(bool copyTopElement)
{
    if (m_parsedBodyElementsStack.empty() == false)
    {
        auto currentElement = m_parsedBodyElementsStack.top();
        m_parsedBodyElementsStack.pop();

        if (currentElement and currentElement->hasValidContent())
        {
            currentElement->finalize();
        }
    }
    //
    // Below creates a copy of the parent and adds to m_content collection on "end" of child element.
    //
    // This is to cover cases like:
    // <p><span xml:id="span1">2</span><br/><span xml:id="span2">4</span></p>
    // "p" element is parent, "span" elements are children of "p". Whenever end of "span" element is detected -
    // this function is called for "span" - a copy of "p" is added to m_content collection and endElement() is called
    // for previously added "p" - so there is single "p" element on BodyElement-s stack.
    //
    // <br/> - new line - needs to be added in the correct place - after 2
    // To achieve this <p> element is added to m_content collection each time its child element ends.
    // Once it's added as top element - the text and new lines are added to it (by the Parser).
    //
    // With adding <p> element on child end rendering is:
    // 2<line break>4
    //
    // Without adding <p> element on child end rendering is:
    // 24<line break>
    //
    // As a side effect following <p> is now correctly supported as well:
    // <p>1<span>2</span>3<span>4</span>5</p>
    //
    if (copyTopElement && m_parsedBodyElementsStack.empty() == false)
    {
        //Note: copyElement is created with its original element as parent
        auto copyElement = std::make_shared<BodyElement>(m_parsedBodyElementsStack.top());
        endElement(false);
        m_parsedBodyElementsStack.push(copyElement);
        m_content.push_back(copyElement);
    }

    m_currentImageElement = std::shared_ptr<ImageElement>();
}

std::shared_ptr<Element> DocumentInstance::getCurrentElement() const
{
    return m_parsedBodyElementsStack.empty() ? nullptr : m_parsedBodyElementsStack.top();
}

std::shared_ptr<Element> DocumentInstance::getCurrentImageElement() const
{
    return m_currentImageElement;
}

void DocumentInstance::newEntity(std::vector<IntermediateDocument::Entity>& entities,
               std::string reason) const
{
    if (!entities.empty() && !entities.back().m_textLines.empty())
    {
        applyWhitespaceHandling(entities.back().m_textLines.back());
    }
    entities.emplace_back();
    entities.back().m_region = std::make_shared<RegionElement>();
    m_logger.ostrace(__LOGGER_FUNC__, " - ", reason);
}

void DocumentInstance::applyWhitespaceHandling(IntermediateDocument::TextLine& textLine) const
{
    //Text chunks with DEFAULT space handling are already treated by
    //BodyElement::applyDefaultWhitespaceHandling()
    //now spaces:
    // - leading
    // - trailing
    // - on the border between text chunks
    // need to be handled
    //(they might come from different elements + have different space handling)
    if (!textLine.empty())
    {
        //Remove leading space
        auto& firstChunk = textLine.front();
        if (firstChunk.m_whitespaceHandling == XmlSpace::DEFAULT && !firstChunk.m_text.empty())
        {
            if (isSpace(firstChunk.m_text.front()))
            {
                firstChunk.m_text.erase(0, 1);
            }
        }

        //Remove trailing space
        auto& lastChunk = textLine.back();
        if (lastChunk.m_whitespaceHandling == XmlSpace::DEFAULT && !lastChunk.m_text.empty())
        {
            if (isSpace(lastChunk.m_text.back()))
            {
                lastChunk.m_text.pop_back();
            }
        }

        //Remove spaces on the border between text chunks
        //in case there are mixed (DEFAULT/PRESERVE) whitespace handling
        //check leading and trailing spaces
        for (int i=0; i < ((int)textLine.size() - 1); i++)
        {
            if (textLine[i].m_whitespaceHandling == XmlSpace::DEFAULT && !textLine[i].m_text.empty())
            {
                if (isSpace(textLine[i].m_text.back()) && isSpace(textLine[i+1].m_text.front()))
                {
                    textLine[i].m_text.pop_back();
                }
            }
            if (i > 0 && textLine[i].m_whitespaceHandling == XmlSpace::DEFAULT && !textLine[i].m_text.empty())
            {
                if (isSpace(textLine[i].m_text.front()) && isSpace(textLine[i-1].m_text.back()))
                {
                    textLine[i].m_text.erase(0, 1);
                }
            }
        }
    }
}

void DocumentInstance::newLine(IntermediateDocument::Entity& entity) const
{
    if (!entity.m_textLines.empty())
    {
        applyWhitespaceHandling(entity.m_textLines.back());
    }
    entity.m_textLines.emplace_back();
}

std::list<IntermediateDocument> DocumentInstance::generateTimeline() const
{
    std::list<IntermediateDocument> timeline;

    if (m_root)
    {
        auto timings = generateTimings();
        updateStyleAttributes();
        updateRegionAttributes();

        for (auto &timing : timings)
        {
            m_logger.ostrace(__LOGGER_FUNC__, ' ', timing.toStr());

            std::vector<IntermediateDocument::Entity> entities;

            for (auto &content : m_content)
            {
                assert(content.get() != nullptr);

                if (content->getTiming().isOverlapping(timing))
                {
                    if (entities.empty())
                    {
                        //Add new Entity on timing change
                        newEntity(entities, "document start");
                    }
                    auto region = findRegion(content->getRegionId());
                    if (region != nullptr)
                    {
                        //Create a new entity if the current one is not empty and is not in the same
                        //region.
                        //Otherwise just update the region
                        if ((!entities.back().empty()) &&
                            ((entities.back().m_region == nullptr) ||
                            (entities.back().m_region->getId() != content->getRegionId())))
                        {
                            newEntity(entities, "region");
                        }
                        entities.back().m_region = std::move(region);
                    }

                    auto image = findImage(content->getBackgroundImageId());
                    if (image != nullptr)
                    {
                        //image based subtitles - IntermediateDocument should hold single image
                        entities.back().m_imageChunk.m_image = std::move(image);
                        if (entities.size() > 1)
                        {
                            m_logger.oswarning("Subtitles with multiple images or mixed image and text not supported");
                        }
                    }
                    else
                    {
                        //text based subtitles - IntermediateDocument::Entity represents single region to render
                        // it may consists of more than one BodyElement-s
                        const auto& contextLines = content->getTextLines();

                        for (const auto& textLine : contextLines)
                        {
                            if (!textLine.text.empty())
                            {
                                if (entities.back().m_textLines.empty())
                                {
                                    newLine(entities.back());
                                }

                                auto& currentLine = entities.back().m_textLines.back();
                                currentLine.emplace_back();
                                auto& textChunk = currentLine.back();
                                textChunk.m_text = textLine.text;
                                //textChunk.m_whitespaceHandling = content->getWhiteSpaceHandling();

                                auto styleId = content->getStyleId();
                                textChunk.m_style.setStyleId(styleId);
                                textChunk.m_style.merge(content->getStyleAttributes());

                                m_logger.ostrace(__LOGGER_FUNC__, " chunk: \'", textChunk.m_text, "\'", ", style: ", textChunk.m_style.toStr());
                            }
                            //If TTML contained new line mark - add new line to render
                            if (textLine.isForcedLine == true && !entities.back().empty())
                            {
                                newLine(entities.back());
                            }
                        }
                    }
                }
            }
            if (!entities.empty())
            {
                if (!entities.back().m_textLines.empty())
                {
                    applyWhitespaceHandling(entities.back().m_textLines.back());
                }

                IntermediateDocument timespan;

                timespan.m_entites = std::move(entities);
                timespan.m_timing = timing;
                timespan.setCellResolution(m_root->getCellResolution());

                timeline.emplace_back(std::move(timespan));
            }
        }
    }
    else
    {
        m_logger.oswarning("Root <tt> element not found - ignoring content");
    }

    return timeline;
}

void DocumentInstance::dump() const
{
    m_logger.ostrace("--------- STYLES -----------");
    for (auto &style : m_styles)
    {
        m_logger.ostrace("id: ", style->getId());
    }

    m_logger.ostrace("--------- REGIONS -----------");
    for (auto &region : m_regions)
    {
        m_logger.ostrace(region->toStr());
    }

    m_logger.ostrace("--------- CONTENTS -----------");
    for (auto &content : m_content)
    {
        auto timing = content->getTiming();
        m_logger.ostrace("id:",
                       content->getId(),
                       " ",
                       timing.toStr(),
                       " reg:",
                       content->getRegionId(),
                       " style:",
                       content->getStyleId(),
                       " lines of text: ",
                       content->getTextLines().size());
    }
}

std::shared_ptr<StyleElement> DocumentInstance::findStyle(const std::string& styleId) const
{
    auto retVal = find(styleId, m_styles);
    if (retVal == nullptr) {
        retVal = std::make_shared<StyleElement>();
    }
    return retVal;
}

std::shared_ptr<RegionElement> DocumentInstance::findRegion(const std::string& regionId) const
{
    return find(regionId, m_regions);
}

std::shared_ptr<ImageElement> DocumentInstance::findImage(const std::string& imageId) const
{
    return find(imageId, m_images);
}

std::vector<Timing> DocumentInstance::generateTimings() const
{
    std::set<TimePoint> timepoints;

    // first create sorted list of unique points in time...
    for (auto &content : m_content)
    {
        std::string contentStr;

        for (auto& line : content->getTextLines()) {
            contentStr.append("[");
            contentStr.append(line.text);
                    contentStr.append("]");
        }
        m_logger.ostrace(
                __LOGGER_FUNC__, " some content, with texts[", content->getTextLines().size(), "]: ", contentStr);

        if (!(content->getTextLines().empty() && content->getBackgroundImageId().empty()))
        {
            timepoints.insert(content->getTiming().getStartTimeRef());
            timepoints.insert(content->getTiming().getEndTimeRef());
        }
    }

    // than generate time periods for every neighboring timepoints
    std::vector<Timing> result;
    if (!timepoints.empty())
    {
        m_logger.ostrace(__LOGGER_FUNC__, " size: ", timepoints.size());

        auto it = timepoints.begin();
        auto it2 = timepoints.begin();
        it2++;

        while (it2 != timepoints.end())
        {
            result.emplace_back(*it, *it2);
            it = it2;
            it2++;
        }
    }

    for (auto x : result)
    {
        m_logger.ostrace(__LOGGER_FUNC__, " result: ", x.toStr());
    }

    return result;
}

void DocumentInstance::updateStyleAttributes() const
{
    for (auto& content : m_content)
    {
        Attributes styleAttrs;

        assert(content.get() != nullptr);
        const auto& parent = content->getParent();

        if (parent != nullptr)
        {
            //merge style attributes from parent
            mergeAttributes(styleAttrs, parent->getStyleAttributes());
        }

        const auto& style = findStyle(content->getStyleId());
        //merge style attributes from element style
        mergeAttributes(styleAttrs, style->getStyleAttributes());

        const auto& region = findRegion(content->getRegionId());
        if (region != nullptr)
        {
            const auto& regionStyle = findStyle(region->getStyleId());
            //merge style attributes from region style
            mergeAttributes(styleAttrs, regionStyle->getStyleAttributes());

            //merge style attributes from region
            mergeAttributes(styleAttrs, region->getStyleAttributes());
        }

        //finally merge style attributes from element
        mergeAttributes(styleAttrs, content->getStyleAttributes());

        // and at last merge style attributes from override-styling
        mergeAttributes(styleAttrs, m_overrideStyleAttributes);

        content->set(std::move(styleAttrs));

        std::ostringstream stream;
        stream << content->getStyleAttributes();
        m_logger.ostrace(__LOGGER_FUNC__, " content style attributes: ", stream.str());
    }
}

void DocumentInstance::setStyleOverrideAttributes(const Attributes& styleAttributes)
{
    m_overrideStyleAttributes = styleAttributes;
}

void DocumentInstance::updateRegionAttributes() const
{
    for (auto& region : m_regions)
    {
        const auto& regionStyle = findStyle(region->getStyleId());
        //merge style attributes from region style
        const auto& regionStyleAttrs = regionStyle->getStyleAttributes();
        for (auto& styleAttr : regionStyleAttrs) {
            region->addAttribute(styleAttr.first, styleAttr.second);
        }

        m_logger.ostrace(__LOGGER_FUNC__, " region: ", region->toStr());
    }
}

} // namespace subttxrend
} // namespace ttmlengine
