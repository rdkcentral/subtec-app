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


#ifndef DVBSUBDECODER_DECODERIMPL_HPP_
#define DVBSUBDECODER_DECODERIMPL_HPP_

#include "Decoder.hpp"
#include "Parser.hpp"
#include "Presenter.hpp"
#include "Storage.hpp"
#include "Types.hpp"

namespace dvbsubdecoder
{

class Allocator;
class DecoderClient;
class TimeProvider;

/**
 * DVB subtitles decoder implementation.
 */
class DecoderImpl : public Decoder
{
    DecoderImpl(const DecoderImpl&) = delete;
    DecoderImpl& operator =(const DecoderImpl&) = delete;

public:
    /**
     * Constructor.
     *
     * @param specVersion
     *      Specification version to support.
     * @param allocator
     *      User memory allocator.
     * @param client
     *      Interface to communicate with client.
     * @param timeProvider
     *      Time (STC) provider.
     */
    DecoderImpl(Specification specVersion,
                Allocator& allocator,
                DecoderClient& client,
                TimeProvider& timeProvider);

    /**
     * Destructor.
     */
    virtual ~DecoderImpl();

    /** @copydoc Decoder::setPageIds */
    virtual void setPageIds(std::uint16_t compositionPageId,
                            std::uint16_t ancillaryPageId) override;

    /** @copydoc Decoder::start */
    virtual void start() override;

    /** @copydoc Decoder::stop */
    virtual void stop() override;

    /** @copydoc Decoder::reset */
    virtual void reset() override;

    /** @copydoc Decoder::addPesPacket */
    virtual bool addPesPacket(const std::uint8_t* buffer,
                              std::size_t size) override;

    /** @copydoc Decoder::invalidate */
    virtual void invalidate() override;

    /** @copydoc Decoder::draw */
    virtual void draw() override;

    /** @copydoc Decoder::process */
    virtual bool process() override;

private:
    /** Interface to communicate with client. */
    DecoderClient& m_client;

    /** Time provider. */
    TimeProvider& m_timeProvider;

    /** Data storage. */
    Storage m_storage;

    /** Presenter for drawing subtitles. */
    Presenter m_presenter;

    /** Parser for decoding data. */
    Parser m_parser;

    /** Decoder started flag. */
    bool m_started;

    /** Redraw needed flag. */
    bool m_redrawNeeded;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_DECODERIMPL_HPP_*/
