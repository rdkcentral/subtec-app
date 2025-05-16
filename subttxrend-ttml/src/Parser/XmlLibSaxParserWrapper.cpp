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

#include "XmlLibSaxParserWrapper.hpp"
#include <cassert>

namespace subttxrend
{
namespace ttmlengine
{

SaxParser::SaxParser(SaxCallbacks& callbacks) :
        m_saxHandler(), m_parserCtxPtr(nullptr), m_saxCallbacks(callbacks), m_logger("TtmlEngine", "SaxParser")
{
    m_logger.ostrace(__LOGGER_FUNC__);
    init();
}

SaxParser::~SaxParser()
{
    cleanup();
}

void SaxParser::parse(const std::uint8_t* buffer,
                std::size_t size)
{
    static constexpr int LIBXML_PARSE_TERMINATION_MARKER = 1;
    auto parseResult = xmlParseChunk(m_parserCtxPtr,
                                     reinterpret_cast<const char*>(buffer),
                                     static_cast<int>(size),
                                     LIBXML_PARSE_TERMINATION_MARKER);
    if (parseResult != 0)
    {
        m_logger.oserror(__LOGGER_FUNC__, " error parsing chunk: ", parseResult);
    }
}

void SaxParser::reset()
{
    cleanup();
    init();
}

void SaxParser::init()
{
    assert(m_parserCtxPtr == nullptr);

    m_saxHandler.initialized = XML_SAX2_MAGIC;

    m_saxHandler.startDocument = onStartDocument;
    m_saxHandler.endDocument = onEndDocument;

    m_saxHandler.startElementNs = onStartElementNs;
    m_saxHandler.endElementNs = onEndElementNs;

    m_saxHandler.characters = onCharacters;

    m_saxHandler.warning = onWarning;
    m_saxHandler.error = onError;

    m_parserCtxPtr = xmlCreatePushParserCtxt(&m_saxHandler, this, nullptr, 0, nullptr);
}

void SaxParser::cleanup()
{
    if (m_parserCtxPtr)
    {
        xmlFreeParserCtxt(m_parserCtxPtr);
        m_parserCtxPtr = nullptr;
    }
    m_saxHandler = xmlSAXHandler();
}

SaxParser* SaxParser::cast(void* ptr)
{
    return reinterpret_cast<SaxParser*>(ptr);
}

void SaxParser::onStartDocument(void *ctx)
{
    auto thiz = cast(ctx);
    thiz->m_saxCallbacks.onStartDocument();
}

void SaxParser::onEndDocument(void *ctx)
{
    auto thiz = cast(ctx);
    thiz->m_saxCallbacks.onEndDocument();
}

void SaxParser::onStartElementNs(void * ctx,
                             const xmlChar * localname,
                             const xmlChar * prefix,
                             const xmlChar * URI,
                             int nb_namespaces,
                             const xmlChar ** namespaces,
                             int nb_attributes,
                             int nb_defaulted,
                             const xmlChar ** attributes)
{
    auto thiz = cast(ctx);

    std::vector<SaxCallbacks::Attribute> attributesVector{static_cast<std::size_t>(nb_attributes)};
    unsigned int index = 0;
    for (int indexAttribute = 0; indexAttribute < nb_attributes; ++indexAttribute, index += 5)
    {
        // name should always be present
        assert(attributes[index]);
        std::string attrName = std::string(reinterpret_cast<const char*>(attributes[index]));

        // prefix is optional
        auto attrPrefixPtr = reinterpret_cast<const char*>(attributes[index + 1]);
        std::string attrPrefix = std::string(attrPrefixPtr ? attrPrefixPtr : "");

        // value should be present
        assert(attributes[index + 3]);
        assert(attributes[index + 4]);
        auto valueBegin = reinterpret_cast<const char*>(attributes[index + 3]);
        auto valueEnd = reinterpret_cast<const char*>(attributes[index + 4]);
        std::string value(valueBegin, valueEnd);

        attributesVector.emplace_back(SaxCallbacks::Attribute{attrPrefix, attrName, value});
    }

    assert(localname != nullptr);
    thiz->m_saxCallbacks.onStartElementNs(reinterpret_cast<const char*>(localname),
        prefix != nullptr ? reinterpret_cast<const char*>(prefix) : "",
        URI != nullptr ? reinterpret_cast<const char*>(URI) : "",
        attributesVector);
}

void SaxParser::onEndElementNs(void * ctx,
                           const xmlChar * localname,
                           const xmlChar * prefix,
                           const xmlChar * URI)
{
    auto thiz = cast(ctx);
    assert(localname != nullptr);
    thiz->m_saxCallbacks.onEndElementNs(reinterpret_cast<const char*>(localname),
        prefix != nullptr ? reinterpret_cast<const char*>(prefix) : "",
        URI != nullptr ? reinterpret_cast<const char*>(URI) : "");
}

void SaxParser::onError(void * ctx,
                    const char * msg,
                    ...)
{
    auto thiz = cast(ctx);
    thiz->m_saxCallbacks.onError(msg);
}

void SaxParser::onWarning(void * ctx,
                      const char * msg,
                      ...)
{
    auto thiz = cast(ctx);
    thiz->m_saxCallbacks.onWarning(msg);
}

void SaxParser::onCharacters(void *ctx,
                         const xmlChar *ch,
                         int len)
{
    auto thiz = cast(ctx);
    thiz->m_saxCallbacks.onCharacters(std::string{ch, ch + len});
}


} //namespace subttxrend
} // namespace ttmlengine
