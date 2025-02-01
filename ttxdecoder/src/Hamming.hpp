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


#ifndef TTXDECODER_HAMMING_HPP_
#define TTXDECODER_HAMMING_HPP_

#include <cstdint>

namespace ttxdecoder
{

/**
 * Hamming code decoder.
 */
class Hamming
{
public:
    /**
     * Decodes Hamming 24/18 code.
     *
     * @param byte1
     *      First transmitted byte.
     * @param byte2
     *      Second transmitted byte.
     * @param byte3
     *      Third transmitted byte.
     *
     * @return
     *      Decoded value (greater or equal to zero).
     *      Negative on errors.
     */
    std::int32_t decode2418(std::uint8_t byte1,
                            std::uint8_t byte2,
                            std::uint8_t byte3);

    /**
     * Decodes Hamming 8/4 code.
     *
     * @param byte1
     *      Transmitted byte.
     *
     * @return
     *      Decoded value (greater or equal to zero).
     *      Negative on errors.
     */
    std::int8_t decode84(std::uint8_t byte1);

    /**
     * Decodes parity protected char.
     *
     * @param byte1
     *      Character to decode.
     *
     * @return
     *      Decoded value (greater or equal to zero).
     *      Negative on errors.
     */
    std::int8_t decodeParity(std::uint8_t byte1);
};

} // namespace ttxdecoder

#endif /*TTXDECODER_HAMMING_HPP_*/
