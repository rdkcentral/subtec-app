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


#ifndef DVBSUBDECODER_DECODERFACTORY_HPP_
#define DVBSUBDECODER_DECODERFACTORY_HPP_

#include <memory>

#include "Decoder.hpp"
#include "Allocator.hpp"
#include "DecoderClient.hpp"
#include "TimeProvider.hpp"
#include "Types.hpp"

namespace dvbsubdecoder
{

/** Unique pointer to decoder. */
using DecoderPtr = Allocator::UniquePtr<Decoder>;

/**
 * Factory for decoders.
 */
class DecoderFactory
{
    DecoderFactory() = delete;

public:
    /**
     * Creates the decoder.
     *
     * Created decoder and internal buffers (excluding pixmap buffers)
     * are allocated using the specified allocator. The allocator and allocated
     * memory blocks must remain valid for the whole lifecycle of created
     * decoder.
     *
     * The implementation will allocate all memory at the moment of decoder
     * creation and release on decoder release. There should be no memory
     * allocations during the decoder operation.
     *
     * @param specVersion
     *      Specification version.
     * @param allocator
     *      Allocator to be used.
     * @param client
     *      Interface to communicate with client.
     * @param timeProvider
     *      Time provider interface.
     *
     * @return
     *      Created decoder.
     *
     * @throw std::bad_alloc
     *      If there is not enough memory to create the decoder.
     */
    static DecoderPtr create(Specification specVersion,
                             Allocator& allocator,
                             DecoderClient& client,
                             TimeProvider& timeProvider);

};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_DECODERFACTORY_HPP_*/
