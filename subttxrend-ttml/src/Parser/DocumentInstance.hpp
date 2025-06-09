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
    DocumentInstance();

    /**
     * Clears document.
     */
    void reset();

    /**
     * Adds element to the document.
     *
     * @param name
     *      Element name.
     * @return
     *      Shared pointer to the element or nullptr if element type is not supported.
     */
    std::shared_ptr<Element> startElement(const std::string& name);

    /**
     * Sets styling override attributes.
     *
     * @param styleAttributes
     *      The styling override attributes.
     */
    void setStyleOverrideAttributes(const Attributes& styleAttributes);

    /**
     * Signals end of element.
     */
    void endElement(bool copyTopElement = true);

    /**
     * Currently parsed element getter.
     *
     * @return
     *      Currently parsed element.
     */
    std::shared_ptr<Element> getCurrentElement() const;

    std::shared_ptr<Element> getCurrentImageElement() const;

    void newEntity(std::vector<IntermediateDocument::Entity>& entities,
                   std::string reason) const;

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
    std::list<IntermediateDocument> generateTimeline() const;

    /**
     * Dumps contents to log. Used for debugging purposes.
     */
    void dump() const;

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
    std::shared_ptr<StyleElement> findStyle(const std::string& styleId) const;

    /**
     * Finds region with given region id.
     *
     * @param regionId
     *      RegionId of region to find.
     * @return
     *      RegionElement instance with given id, nullptr if not found.
     */
    std::shared_ptr<RegionElement> findRegion(const std::string& regionId) const;

    /**
     * Finds image with given id.
     *
     * @param imageId
     *      Image id to find.
     * @return
     *      ImageElement instance with given id, nullptr if not found.
     */
    std::shared_ptr<ImageElement> findImage(const std::string& imageId) const;

    /**
     * Generates list of time periods within which subtitle content does not change.
     *
     * @return
     *      Sorted list of time periods.
     */
    std::vector<Timing> generateTimings() const;

    /**
     * Update style attributes for all BodyElement-s
     * Order of style resolution:
     * - parent
     * - style
     * - region style
     * - region
     * - "self"
     */
    void updateStyleAttributes() const;

    /**
     * Add style attributes referenced by the region into region
     * As a result, all attributes referenced by style become attributes of region
     * among others "tts:extent" and "tts:origin"
     */
    void updateRegionAttributes() const;

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

    /** The styling attributes to use as overrides for all other styles */
    Attributes m_overrideStyleAttributes;

    /** Logger object. */
    mutable subttxrend::common::Logger m_logger;
};

} // namespace subttxrend
} // namespace ttmlengine

#endif /* SUBTTXREND_TTML_PARSER_DOCUMENTINSTANCE_HPP_ */

