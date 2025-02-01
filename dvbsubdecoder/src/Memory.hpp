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


#ifndef DVBSUBDECODER_MEMORY_HPP_
#define DVBSUBDECODER_MEMORY_HPP_

#include <cstdint>

namespace dvbsubdecoder
{

/**
 * Aligns the pointer to memory block.
 *
 * It does same thing as std::align which is unfortunately missing in GCC 4.9.
 *
 * @param alignment
 *      Requested alignment.
 * @param size
 *      Requested size.
 * @param ptr
 *      Pointer to memory block (would be shifted to right position on success).
 * @param space
 *      Size of the memory block.
 *
 * @return
 *      Aligned pointer on success or nullptr on error.
 *
 * align function based on work of David Krauss
 * https://gcc.gnu.org/bugzilla/show_bug.cgi?id=57350
 * MIT Licensed code
 *
 */
inline void* align(std::size_t alignment,
                   std::size_t size,
                   void *&ptr,
                   std::size_t &space)
{
    std::uintptr_t pn = reinterpret_cast<std::uintptr_t>(ptr);
    std::uintptr_t aligned = (pn + alignment - 1) & -alignment;
    std::size_t padding = aligned - pn;
    if (space < size + padding)
        return nullptr;
    space -= padding;
    return ptr = reinterpret_cast<void *>(aligned);
}

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_MEMORY_HPP_*/
