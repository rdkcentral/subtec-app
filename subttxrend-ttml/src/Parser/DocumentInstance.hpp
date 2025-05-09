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


#ifndef SUBTTXREND_TTML_PARSER_DOCUMENTINSTANCE_HPP_
#define SUBTTXREND_TTML_PARSER_DOCUMENTINSTANCE_HPP_

#include <list>
#include <memory>
#include <stack>

#include <subttxrend/common/Logger.hpp>

#include "Elements.hpp"
#include "IntermediateDocument.hpp"

namespace subttxrend
{
namespace ttmlengine
{

/**
 * Class representing Ttml document.
 */
class DocumentInstance
{
public:

    /**
     * Constructor.
     */
    DocumentInstance() :
            m_styles(),
            m_regions(),
            m_parsedBodyElementsStack(),
            m_content(),
            m_logger("TtmlEngine", "DocumentInstance")
    {
        // noop
    }

    /**
     * Clears document.
     */
    void reset()
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

        m_currentImageElement = std::shared_ptr<ImageElement>();
    }

    /**
     * Adds element to the document.
     *
     * @param name
     *      Element name.
     * @return
     *      Shared pointer to the element or nullptr if element type is not supported.
     */
    std::shared_ptr<Element> startElement(const std::string& name)
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

    /**
     * Signals end of element.
     */
    void endElement(bool copyTopElement = true)
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

    /**
     * Currently parsed element getter.
     *
     * @return
     *      Currently parsed element.
     */
    std::shared_ptr<Element> getCurrentElement() const
    {
        return m_parsedBodyElementsStack.empty() ? nullptr : m_parsedBodyElementsStack.top();
    }

    std::shared_ptr<Element> getCurrentImageElement() const
    {
        return m_currentImageElement;
    }

    void newEntity(std::vector<IntermediateDocument::Entity>& entities,
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

    void applyWhitespaceHandling(IntermediateDocument::TextLine& textLine) const
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

    void newLine(IntermediateDocument::Entity& entity) const
    {
        if (!entity.m_textLines.empty())
        {
            applyWhitespaceHandling(entity.m_textLines.back());
        }
        entity.m_textLines.emplace_back();
    }

    /**
     * Generates list of time periods during which ttml content is constant.
     *
     * The algorithm goes through parsed BodyElement-s and based on that creates IntermediateDocument-s.
     * For text based TTMLs each IntermediateDocument holds content to be printed within given timespan.
     * Each IntermediateDocument::Entiy holds single region,
     * each IntermediateDocument::TextLine holds single line,
     * each IntermediateDocument::TextChunk holds single text chunk
     * to be printed on the screen.
     * IntermediateDocument may consists of more than one BodyElement.
     * New IntermediateDocument::Entity is created when given BodyElement has it's own region defined.
     * New IntermediateDocument::TextLine is created when new line is forced - <br> element.
     *
     * @return
     *      Timelined list of text content and it's properties.
     */
    std::list<IntermediateDocument> generateTimeline() const
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
                            //New region - create new entity if current is not empty
                            //Otherwise just update the region
                            if (!entities.back().empty())
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
                                    textChunk.m_whitespaceHandling = content->getWhiteSpaceHandling();

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

    /**
     * Dumps contents to log. Used for debugging purposes.
     */
    void dump() const
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

private:

    /**
     * Template method for finding items in sets by their id.
     *
     * @param itemId
     *      Item id to find.
     *
     * @return
     *      T type element instance with given id, default element if not found.
     */
    template<typename T>
    std::shared_ptr<T> find(const std::string& itemId, const std::set< std::shared_ptr<T> >& container) const
    {
        auto result = std::find_if(container.begin(),
                                   container.end(),
                                   [&itemId](const std::shared_ptr<T>& elem)
                                   {   return elem->getId() == itemId;});

        if (result != container.end())
        {
            return *result;
        }
        return nullptr;
    }

    /**
     * Finds style with given id.
     *
     * @param styleId
     *      Style id to find.
     * @return
     *      StyleElement instance with given id, default style if not found.
     */
    std::shared_ptr<StyleElement> findStyle(const std::string& styleId) const
    {
        auto retVal = find(styleId, m_styles);
        if (retVal == nullptr) {
            retVal = std::make_shared<StyleElement>();
        }
        return retVal;
    }

    /**
     * Finds region with given region id.
     *
     * @param regionId
     *      RegionId of region to find.
     * @return
     *      RegionElement instance with given id, nullptr if not found.
     */
    std::shared_ptr<RegionElement> findRegion(const std::string& regionId) const
    {
        return find(regionId, m_regions);
    }

    /**
     * Finds image with given id.
     *
     * @param imageId
     *      Image id to find.
     * @return
     *      ImageElement instance with given id, nullptr if not found.
     */
    std::shared_ptr<ImageElement> findImage(const std::string& imageId) const
    {
        return find(imageId, m_images);
    }

    /**
     * Generates list of time periods within which subtitle content does not change.
     *
     * @return
     *      Sorted list of time periods.
     */
    std::vector<Timing> generateTimings() const
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

    /**
     * Update style attributes for all BodyElement-s
     * Order of style resolution:
     * - parent
     * - style
     * - region style
     * - region
     * - "self"
     */
    void updateStyleAttributes() const
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

            content->set(std::move(styleAttrs));

            std::ostringstream stream;
            stream << content->getStyleAttributes();
            m_logger.ostrace(__LOGGER_FUNC__, " content style attributes: ", stream.str());
        }
    }

    /**
     * Add style attributes referenced by the region into region
     * As a result, all attributes referenced by style become attributes of region
     * among others "tts:extent" and "tts:origin"
     */
    void updateRegionAttributes() const
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

private:
    /** TT root element */
    std::shared_ptr<TTElement> m_root;

    /** Set of styles defined in the document. Each style may be referenced by multiple content elements. */
    std::set<std::shared_ptr<StyleElement> > m_styles;

    /** Set of regions defined in the document. Each region may be referenced by multiple content elements. */
    std::set<std::shared_ptr<RegionElement> > m_regions;

    // maps imageid -> image data (base64 encoded string)
    std::set<std::shared_ptr<ImageElement>> m_images;

    /** Current element stack. Used to keep track of current element mainly for children nodes to inherit
     * parent node properties. */
    std::stack<std::shared_ptr<BodyElement> > m_parsedBodyElementsStack;

    std::shared_ptr<ImageElement> m_currentImageElement;

    /** List of found content nodes. */
    std::vector<std::shared_ptr<BodyElement> > m_content;

    /** Logger object. */
    mutable subttxrend::common::Logger m_logger;
};

} // namespace subttxrend
} // namespace ttmlengine

#endif /* SUBTTXREND_TTML_PARSER_DOCUMENTINSTANCE_HPP_ */

