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


#ifndef DVBSUBDECODER_DECODER_HPP_
#define DVBSUBDECODER_DECODER_HPP_

#include <cstddef>
#include <cstdint>

namespace dvbsubdecoder
{

/**
 * DVB subtitles decoder interface.
 */
class Decoder
{
public:
    /**
     * Constructor.
     */
    Decoder() = default;

    /**
     * Destructor.
     */
    virtual ~Decoder() = default;

    /**
     * Sets ancillary/composition page ids to decode.
     *
     * @param compositionPageId
     *      Composition page id.
     * @param ancillaryPageId
     *      Ancillary page id.
     *      If ancillary page should not be used (e.g. because it is not
     *      present in the stream) the value shall be equal to composition
     *      page identifier.
     */
    virtual void setPageIds(std::uint16_t compositionPageId,
                            std::uint16_t ancillaryPageId) = 0;


    /**
     * Starts the decoder.
     *
     * This method could be used to start/restarts decoding of the packets.
     * If decoder is already started does nothing.
     */
    virtual void start() = 0;

    /**
     * Stops the decoder.
     *
     * This method could be used to stop decoding of the packets.
     * If decoder is already stopped does nothing.
     */
    virtual void stop() = 0;

    /**
     * Resets the decoder.
     *
     * The internal PES packet queue is cleared, any already decoded data
     * is dropped and the internal state is reset to default values.
     *
     * It does not resets the page ids set.
     */
    virtual void reset() = 0;

    /**
     * Adds PES packet to be processed.
     *
     * Copies the given PES packet to internal buffer for later processing.
     *
     * @param buffer
     *      Buffer with PES packet.
     * @param size
     *      Size of the PES packet in bytes.
     *
     * @retval true
     *      PES packet was added successfully to the internal buffer.
     * @retval false
     *      There was not enough space for the packet to be added.
     */
    virtual bool addPesPacket(const std::uint8_t* buffer,
                              std::size_t size) = 0;

    /**
     * Invalidates the specified subtitle decoder.
     *
     * This method could be used to inform the decoder that it should
     * invalidate the stored rendering state and redraw all subtitles.
     *
     * @note The method does not guarantee that the whole screen would be
     *       cleared.
     * @note This method does not draw the subtitles. A call to draw() method
     *       is needed to perform rendering.
     */
    virtual void invalidate() = 0;

    /**
     * Draws the subtitles.
     *
     * Requests the decoder to invoke the client callbacks to render the
     * subtitles on-screen.
     */
    virtual void draw() = 0;

    /**
     * Processes the PES packets.
     *
     * Checks if the current STC value matches the Presentation Time Stamp
     * (PTS) of one or more PES packets in the internal PES packet queue,
     * and decodes them. Packets are removed from the buffer after parsing.
     *
     * @retval true
     *      Some data was processed. If there were some new subtitles decoded
     *      draw() should be called to render them.
     * @retval false
     *      No data was processed.
     */
    virtual bool process() = 0;

};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_DECODER_HPP_*/
