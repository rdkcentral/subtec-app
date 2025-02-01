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


#ifndef SUBTTXREND_TTML_PARSER_XMLLIBSAXPARSERWRAPPER_HPP
#define SUBTTXREND_TTML_PARSER_XMLLIBSAXPARSERWRAPPER_HPP

#include <libxml/parser.h>
#include <libxml/SAX.h>

#include <subttxrend/common/Logger.hpp>

#include <cstdint>
#include <vector>

namespace subttxrend
{
namespace ttmlengine
{

/**
 * Callback interface for sax parser.
 */
class SaxCallbacks
{
public:

    struct Attribute
    {
        std::string prefix;
        std::string name;
        std::string value;
    };

    /**
     * Destructor.
     */
    virtual ~SaxCallbacks()
    {
        // noop
    }

    /**
     * Start document callback.
     */
    virtual void onStartDocument() = 0;

    /**
     * End document callback.
     */
    virtual void onEndDocument() = 0;

    /**
     * Start element callback.
     *
     * @param ctx
     *      Context passed - expected to be pointer to class instance.
     * @param localname
     *   Local name of the element.
     * @param prefix
     *      The element namespace prefix if available.
     * @param URI
     *      The element namespace name if available.
     * @param attributes
     *      Attributes vector.
     */
    virtual void onStartElementNs(const std::string& localname,
                                  const std::string& prefix,
                                  const std::string& URI,
                                  const std::vector<Attribute>& attributes) = 0;

    /**
     * SAX2 callback when an element end has been detected by the parser. It provides the namespace informations for the element.
     *
     * @param ctx
     *      The user data (XML parser context).
     * @param localname
     *      The local name of the element.
     * @param prefix
     *      The element namespace prefix if available.
     * @param URI
     *      The element namespace name if available
     */
    virtual void onEndElementNs(const std::string& localname,
                                const std::string& prefix,
                                const std::string& URI) = 0;

    /**
     * Display and format error messages callback.
     *
     * @param ctx
     *      An XML parser context.
     * @param msg
     *      The message to display/transmit.
     */
    virtual void onError(const char * msg,
                         ...) = 0;

    /**
     * Display and format warning messages callback.
     *
     * @param ctx
     *      An XML parser context.
     * @param msg
     *      The message to display/transmit.
     */
    virtual void onWarning(const char * msg,
                           ...) = 0;

    /**
     * Receiving some chars from the parser.
     *
     * @param content
     *      Content string.
     */
    virtual void onCharacters(const std::string& content) = 0;
};

/**
 * Libxml sax parser interface wrapper.
 */
class SaxParser
{
public:

    /**
     * Constructor.
     *
     * @param callbacks
     *      Sax callback receiver.
     */
    SaxParser(SaxCallbacks& callbacks) :
            m_saxHandler(), m_parserCtxPtr(nullptr), m_saxCallbacks(callbacks), m_logger("TtmlEngine", "SaxParser")
    {
        m_logger.ostrace(__LOGGER_FUNC__);
        init();
    }

    /**
     * Destructor.
     */
    ~SaxParser()
    {
        cleanup();
    }

    /**
     *  Parses chunk of data. Assumes buffer contains full xml document.
     *
     * @param buffer
     *      Buffer to parse.
     * @param size
     *      Buffer size.
     */
    void parse(const std::uint8_t* buffer,
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

    /**
     * Resets parser.
     */
    void reset()
    {
        cleanup();
        init();
    }

private:

    /**
     * Initialization function.
     */
    void init()
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

    /**
     * Cleanup function.
     *
     */
    void cleanup()
    {
        if (m_parserCtxPtr)
        {
            xmlFreeParserCtxt(m_parserCtxPtr);
            m_parserCtxPtr = nullptr;
        }
        m_saxHandler = xmlSAXHandler();
    }

    /**
     * Casts pointer to class instance.
     *
     * @param ptr
     *      Pointer to case.
     * @return
     *      Pointer to class instance.
     */
    static SaxParser* cast(void* ptr)
    {
        return reinterpret_cast<SaxParser*>(ptr);
    }

    /**
     * Start document callback.
     *
     * @param ctx
     *      Context passed - expected to be pointer to class instance.
     */
    static void onStartDocument(void *ctx)
    {
        auto thiz = cast(ctx);
        thiz->m_saxCallbacks.onStartDocument();
    }

    /**
     * End document callback.
     *
     * @param ctx
     *      Context passed - expected to be pointer to class instance.
     */
    static void onEndDocument(void *ctx)
    {
        auto thiz = cast(ctx);
        thiz->m_saxCallbacks.onEndDocument();
    }

    /**
     * Start element callback.
     *
     * @param ctx
     *      Context passed - expected to be pointer to class instance.
     * @param localname
     *   Local name of the element.
     * @param prefix
     *      The element namespace prefix if available.
     * @param URI
     *      The element namespace name if available.
     * @param nb_namespaces
     *      Number of namespace definitions on that node.
     * @param namespaces
     *      Pointer to the array of prefix/URI pairs namespace definitions.
     * @param nb_attributes
     *      The number of attributes on that node.
     * @param nb_defaulted
     *      The number of defaulted attributes. The defaulted ones are at the end of the array.
     * @param attributes
     *      Pointer to the array of (localname/prefix/URI/value/end) attribute values.
     */
    static void onStartElementNs(void * ctx,
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

    /**
     * SAX2 callback when an element end has been detected by the parser. It provides the namespace informations for the element.
     *
     * @param ctx
     *      The user data (XML parser context).
     * @param localname
     *      The local name of the element.
     * @param prefix
     *      The element namespace prefix if available.
     * @param URI
     *      The element namespace name if available
     */
    static void onEndElementNs(void * ctx,
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

    /**
     * Display and format error messages callback.
     *
     * @param ctx
     *      An XML parser context.
     * @param msg
     *      The message to display/transmit.
     */
    static void onError(void * ctx,
                        const char * msg,
                        ...)
    {
        auto thiz = cast(ctx);
        thiz->m_saxCallbacks.onError(msg);
    }

    /**
     * Display and format warning messages callback.
     *
     * @param ctx
     *      An XML parser context.
     * @param msg
     *      The message to display/transmit.
     */
    static void onWarning(void * ctx,
                          const char * msg,
                          ...)
    {
        auto thiz = cast(ctx);
        thiz->m_saxCallbacks.onWarning(msg);
    }

    /**
     * Receiving some chars from the parser.
     *
     * @param ctx
     *      The user data (XML parser context).
     * @param ch
     *      A xmlChar string.
     * @param len
     *      the number of xmlChar.
     */
    static void onCharacters(void *ctx,
                             const xmlChar *ch,
                             int len)
    {
        auto thiz = cast(ctx);
        thiz->m_saxCallbacks.onCharacters(std::string{ch, ch + len});
    }

    /** xmlLib sax handler. */
    xmlSAXHandler m_saxHandler;

    /** xmlLib parser context.*/
    xmlParserCtxtPtr m_parserCtxPtr;

    /** Registered callbacks handler. */
    SaxCallbacks& m_saxCallbacks;

    /** Logger object. */
    subttxrend::common::Logger m_logger;
};

} //namespace subttxrend
} // namespace ttmlengine

#endif /* SUBTTXREND_TTML_PARSER_XMLLIBSAXPARSERWRAPPER_HPP */
