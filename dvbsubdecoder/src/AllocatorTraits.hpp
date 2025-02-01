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


#ifndef DVBSUBDECODER_ALLOCATORTRAITS_HPP_
#define DVBSUBDECODER_ALLOCATORTRAITS_HPP_

#include "Allocator.hpp"

namespace dvbsubdecoder
{

/**
 * Allocator traits helper class.
 *
 * This class should be used instead of direct usage of allocator as it
 * provides common allocator operations in a type agnostic manner.
 */
class AllocatorTraits
{
public:
    /**
     * Unique pointer type with custom allocator deleter.
     */
    template<typename TObject>
    using UniquePtr = std::unique_ptr<TObject, Allocator::Deleter<TObject>>;

    /**
     * Constructor.
     *
     * @param allocator
     *      Allocator to use.
     */
    AllocatorTraits(Allocator& allocator) :
            m_allocator(allocator)
    {
        // noop
    }

    /**
     * Allocates object.
     *
     * @param args
     *      Arguments to be passed to constructor.
     *
     * @return
     *      Unique pointer to allocated object.
     *
     * @tparam TObject
     *      Type of object to allocate.
     */
    template<typename TObject, typename ... Args>
    UniquePtr<TObject> allocUnique(Args&&... args)
    {
        void* memoryBlock = nullptr;

        try
        {
            memoryBlock = m_allocator.allocate(sizeof(TObject),
                    alignof(TObject));

            TObject* newObject = new (memoryBlock) TObject(
                    std::forward<Args>(args)...);

            return UniquePtr<TObject>(newObject,
                    typename UniquePtr<TObject>::deleter_type(&m_allocator));
        }
        catch (...)
        {
            if (memoryBlock)
            {
                m_allocator.free(memoryBlock);
            }

            // rethrow
            throw;
        }
    }

private:
    /** Allocator for which traits were created. */
    Allocator& m_allocator;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_ALLOCATORTRAITS_HPP_*/
