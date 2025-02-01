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


#ifndef TTXDECODER_ENGINECLIENT_HPP_
#define TTXDECODER_ENGINECLIENT_HPP_

namespace ttxdecoder
{

/**
 * Engine client interface.
 */
class EngineClient
{
public:
    /**
     * Constructor.
     */
    EngineClient() = default;

    /**
     * Destructor.
     */
    virtual ~EngineClient() = default;

    /**
     * Called when new page data is ready.
     */
    virtual void pageReady() = 0;

    /**
     * Called when new header data is ready.
     */
    virtual void headerReady() = 0;

    /**
     * Called when DRCS char was decoded.
     *
     * @param index
     *      Character index.
     * @param data
     *      Character data.
     */
    virtual void drcsCharDecoded(unsigned char index,
                                 unsigned char* data) = 0;

    /**
     * Called to get current STC value.
     *
     * @param stc
     *      Variable to store the STC value.
     *
     * @retval true
     *      Value was available,
     * @retval false
     *      Value cannot be returned.
     */
    virtual bool getStc(std::uint32_t& stc) = 0;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_ENGINECLIENT_HPP_*/
