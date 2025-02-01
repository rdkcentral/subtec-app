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


#ifndef SUBTTXREND_TTML_PARSER_PARSER_HPP
#define SUBTTXREND_TTML_PARSER_PARSER_HPP

#include <memory>
#include <queue>
#include <vector>

#include "DocumentInstance.hpp"
#include "XmlLibSaxParserWrapper.hpp"
#include <subttxrend/common/Logger.hpp>

namespace subttxrend
{
namespace ttmlengine
{

/**
 * Ttml parser. Uses libxml2 sax parser to generate timelined list ttml subtitles.
 */
class Parser : private SaxCallbacks
{
public:

    /**
     * Constructor.
     */
    Parser() :
            m_saxParser(*this), m_logger("TtmlEngine", "Parser")
    {
        // noop
    }

    /**
     * Destructor.
     */
    ~Parser() = default;

    /**
     * Parses given buffer.
     *
     * @param buffer
     *      Buffer pointer.
     * @param bufferSize
     *      Buffer size.
     */
    std::list<IntermediateDocument> parse(const std::uint8_t* buffer, std::size_t bufferSize);

    /**
     * Resets parser.
     */
    void reset()
    {
        m_saxParser.reset();
        m_docInstance.reset();
        m_timeline.clear();
    }

private:

    /** @copydoc SaxCallbacks::onStartDocument */
    virtual void onStartDocument() override;

    /** @copydoc SaxCallbacks::onEndDocument */
    virtual void onEndDocument() override;

    /** @copydoc SaxCallbacks::onStartElementNs */
    virtual void onStartElementNs(const std::string& localname,
                                  const std::string& prefix,
                                  const std::string& URI,
                                  const std::vector<Attribute>& attributes) override;

    /** @copydoc SaxCallbacks::onEndElementNs */
    virtual void onEndElementNs(const std::string&  localname,
                                const std::string&  prefix,
                                const std::string&  URI) override;

    /** @copydoc SaxCallbacks::onError */
    virtual void onError(const char * msg,
                         ...) override;

    /** @copydoc SaxCallbacks::onWarning */
    virtual void onWarning(const char * msg,
                           ...) override;

    /** @copydoc SaxCallbacks::onCharacters */
    virtual void onCharacters(const std::string& ch) override;

    /**
     * Parses element attributes.
     *
     * @param element
     *      Element.
     * @param attributes
     *      Attributes vector.
     */
    void parseAttributes(Element* element,
                         const std::vector<SaxCallbacks::Attribute>& attributes);

    /**
     * Prints current timeline to the log. Used for debugging purposes.
     *
     * @param timeline
     *      Timeline to print contents of.
     */
    void dumpTimeline(const std::list<IntermediateDocument>& timeline);

   /**
    *
    * Expands current timeline with passed one.
    *
    * @param newTimeline
    *       Timeline to merge.
    */
    void expandTimeline(std::list<IntermediateDocument> newTimeline);

    /**
     * Shrinks timeline to requested size.
     *
     * @param timeline
     *      Timeline to shrink.
     * @param requestedSize
     *      Size to shrink list to.
     */
    void shrinkTimelineTo(std::list<IntermediateDocument>& timeline, std::size_t requestedSize);

    /** Maximum number of timeline entries. If this number is exceed old entries are dropped. */
    static constexpr std::size_t MAX_TIMELINE_SIZE = 1000;

    /** xml sax parser. */
    SaxParser m_saxParser;

    /** Document instance */
    DocumentInstance m_docInstance;
    std::list<IntermediateDocument> m_timeline;

    /** Logger object. */
    subttxrend::common::Logger m_logger;
};

} // namespace subttxrend
} // namespace ttmlengine

#endif /* SUBTTXREND_TTML_PARSER_PARSER_HPP */
