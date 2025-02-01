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


#ifndef DVBSUBDECODER_OBJECTINSTANCE_HPP_
#define DVBSUBDECODER_OBJECTINSTANCE_HPP_

#include <cstdint>
#include <stdexcept>

namespace dvbsubdecoder
{

class ObjectInstance;
class ObjectInstanceList;

/**
 * Object reference - list node.
 */
class ObjectInstanceListNode
{
    friend class ObjectInstanceList;

public:
    /**
     * Constructor.
     */
    ObjectInstanceListNode() :
            m_list(nullptr),
            m_next(nullptr),
            m_prev(nullptr)
    {
        // noop
    }

private:
    /** List pointer. */
    ObjectInstanceList* m_list;

    /** Next list element. */
    ObjectInstance* m_prev;

    /** Previous list element. */
    ObjectInstance* m_next;
};

/**
 * Object reference.
 */
class ObjectInstance : public ObjectInstanceListNode
{
public:
    /** Object identifier. */
    std::uint16_t m_objectId;

    /** Object position X. */
    std::int32_t m_positionX;

    /** Object positon Y. */
    std::int32_t m_positionY;
};

/**
 * List of object references.
 */
class ObjectInstanceList
{
public:
    ObjectInstanceList() :
            m_first(nullptr),
            m_last(nullptr)
    {
        // noop
    }

    /**
     * Adds object to list.
     *
     * @param reference
     *      Object to be added.
     */
    void add(ObjectInstance* reference)
    {
        if (!reference)
        {
            throw std::invalid_argument("reference");
        }
        if (reference->m_list)
        {
            throw std::logic_error("Reference already in list");
        }

        if (!m_first)
        {
            reference->m_next = nullptr;
            reference->m_prev = nullptr;

            m_first = reference;
            m_last = reference;
        }
        else
        {
            reference->m_next = nullptr;
            reference->m_prev = m_last;

            m_last->m_next = reference;
            m_last = reference;
        }

        reference->m_list = this;
    }

    /**
     * Removes first object from list.
     *
     * @return
     *      Removed object or nullptr if list was empty.
     */
    ObjectInstance* removeFirst()
    {
        if (m_first)
        {
            auto reference = m_first;

            m_first = m_first->m_next;
            if (m_first)
            {
                m_first->m_prev = nullptr;
            }
            else
            {
                m_last = nullptr;
            }

            // prev is already null
            reference->m_next = nullptr;
            reference->m_list = nullptr;

            return reference;
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * Returns first object in list.
     *
     * @return
     *      First object or nullptr if list is empty.
     */
    const ObjectInstance* getFirst() const
    {
        return m_first;
    }

    /**
     * Returns next object in list.
     *
     * @param prev
     *      Previous object.
     *
     * @return
     *      Next object or nullptr if there are no more objects.
     */
    const ObjectInstance* getNext(const ObjectInstance* prev) const
    {
        if (!prev)
        {
            throw std::invalid_argument("prev");
        }
        if (prev->m_list != this)
        {
            throw std::logic_error("Reference prev is not in this list");
        }

        return prev->m_next;
    }

private:
    /** First object in list. */
    ObjectInstance* m_first;

    /** Last object in list. */
    ObjectInstance* m_last;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_OBJECTINSTANCE_HPP_*/
