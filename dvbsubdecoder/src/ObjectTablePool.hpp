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


#ifndef DVBSUBDECODER_OBJECTTABLEPOOL_HPP_
#define DVBSUBDECODER_OBJECTTABLEPOOL_HPP_

#include <array>
#include <stdexcept>

namespace dvbsubdecoder
{

/**
 * Object pool and collection.
 *
 * @param TObject
 *      Type of object.
 * @param TId
 *      Type of ID.
 * @param SIZE
 *      Size of the pool (number of objects).
 */
template<class TObject, typename TId, std::size_t SIZE>
class ObjectTablePool
{
public:
    /**
     * Constructor.
     *
     * Constructs pool with no objects.
     */
    ObjectTablePool()
    {
        m_usedCount = 0;
        reset();
    }

    /**
     * Checks if object can be added.
     *
     * @return
     *      True if object can be added, false otherwise.
     */
    bool canAdd() const
    {
        return m_usedCount < SIZE;
    }

    /**
     * Returns number of objects currently used.
     *
     * @return
     *      Number of objects in use.
     */
    std::size_t getCount() const
    {
        return m_usedCount;
    }

    /**
     * Returns object by identifier.
     *
     * @param id
     *      Identifier of the object.
     *
     * @return
     *      Pointer to object if found, nullptr otherwise.
     */
    TObject* getById(TId id)
    {
        for (std::size_t i = 0; i < m_usedCount; ++i)
        {
            if (m_objects[i].getId() == id)
            {
                return &m_objects[i];
            }
        }

        return nullptr;
    }

    /**
     * Returns object by index.
     *
     * @param index
     *      index of the object.
     *
     * @return
     *      Pointer to object.
     */
    TObject* getByIndex(std::size_t index)
    {
        if (index >= m_usedCount)
        {
            throw std::range_error("index");
        }
        return &m_objects[index];
    }

    /**
     * Adds object to pool (allocated new object).
     *
     * @param id
     *      ID of the object to add.
     *
     * @return
     *      Pointer to added (allocated) object or nullptr if there
     *      are no more objects in pool.
     */
    TObject* add(TId id)
    {
        if (getById(id))
        {
            throw std::logic_error("Duplicated id");
        }

        if (canAdd())
        {
            auto& object = m_objects[m_usedCount++];
            object.setId(id);
            return &object;
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * Resets the pool.
     *
     * All the objects added are returned to pool and reset.
     */
    void reset()
    {
        for (std::size_t i = 0; i < m_usedCount; ++i)
        {
            m_objects[i].reset();
        }
        m_usedCount = 0;
    }

private:
    /** Array with objects. */
    std::array<TObject, SIZE> m_objects;

    /** Number of currently used entries. */
    std::size_t m_usedCount;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_OBJECTTABLEPOOL_HPP_*/
