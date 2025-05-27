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
#include <string>
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
    SaxParser(SaxCallbacks& callbacks);

    /**
     * Destructor.
     */
    ~SaxParser();

    /**
     *  Parses chunk of data. Assumes buffer contains full xml document.
     *
     * @param buffer
     *      Buffer to parse.
     * @param size
     *      Buffer size.
     */
    void parse(const std::uint8_t* buffer,
                    std::size_t size);

    /**
     * Resets parser.
     */
    void reset();

private:

    /**
     * Initialization function.
     */
    void init();

    /**
     * Cleanup function.
     *
     */
    void cleanup();

    /**
     * Casts pointer to class instance.
     *
     * @param ptr
     *      Pointer to case.
     * @return
     *      Pointer to class instance.
     */
    static SaxParser* cast(void* ptr);

    /**
     * Start document callback.
     *
     * @param ctx
     *      Context passed - expected to be pointer to class instance.
     */
    static void onStartDocument(void *ctx);

    /**
     * End document callback.
     *
     * @param ctx
     *      Context passed - expected to be pointer to class instance.
     */
    static void onEndDocument(void *ctx);

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
                                 const xmlChar ** attributes);

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
                               const xmlChar * URI);

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
                        ...);

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
                          ...);

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
                             int len);

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
