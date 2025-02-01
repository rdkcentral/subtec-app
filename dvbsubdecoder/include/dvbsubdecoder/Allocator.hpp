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


#ifndef DVBSUBDECODER_ALLOCATOR_HPP_
#define DVBSUBDECODER_ALLOCATOR_HPP_

#include <stdexcept>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace dvbsubdecoder
{

/**
 * Allocator base class.
 */
class Allocator
{
public:
    /**
     * Deleter functor.
     *
     * This functor could be used e.g. with unique_ptr.
     */
    template<class T>
    class Deleter
    {
        template<class OT>
        friend class Deleter;

    public:
        /**
         * Constructor.
         *
         * Creates uninitialized deleter.
         */
        template<class OT>
        Deleter(const Deleter<OT>& other) :
                m_allocator(other.m_allocator)
        {
            static_assert(std::is_convertible<OT*,T*>::value, "Cannot convert pointer type");
        }

        /**
         * Constructor.
         *
         * Creates uninitialized deleter.
         */
        Deleter() :
                m_allocator(nullptr)
        {
            // noop
        }

        /**
         * Constructor.
         *
         * @param allocator
         *      Allocator for which deleter is created.
         */
        Deleter(Allocator* allocator) :
                m_allocator(allocator)
        {
            // noop
        }

        /**
         * Callback - releases the object.
         *
         * @param object
         *      Object to be released.
         */
        void operator ()(T* object)
        {
            if (object)
            {
                if (m_allocator)
                {
                    object->~T();
                    m_allocator->free(object);
                }
                else
                {
                    throw std::logic_error("Uninitialized deleter used");
                }
            }
        }

    private:
        /** Allocator for which deleter is created. */
        Allocator* m_allocator;
    };

    /**
     * Unique pointer type with custom allocator deleter.
     */
    template<class T>
    using UniquePtr = std::unique_ptr<T, Allocator::Deleter<T>>;

    /**
     * Constructor.
     */
    Allocator() = default;

    /**
     * Destructor.
     */
    virtual ~Allocator() = default;

    /**
     * Allocates memory block.
     *
     * @param size
     *      Requested memory block size.
     * @param alignment
     *      Requested memory block alignment.
     *
     * @return
     *      Pointer to allocated memory block.
     *
     * @throw std::bad_alloc
     *      if memory cannot be allocated.
     */
    virtual void* allocate(std::size_t size,
                           std::size_t alignment) = 0;

    /**
     * Releases memory block.
     *
     * @param block
     *      Previously allocated memory block.
     */
    virtual void free(void* block) = 0;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_ALLOCATOR_HPP_*/
