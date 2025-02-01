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


#ifndef DVBSUBDECODER_OBJECTPOOL_HPP_
#define DVBSUBDECODER_OBJECTPOOL_HPP_

#include <array>
#include <cassert>

namespace dvbsubdecoder
{

/**
 * Object pool.
 */
template<class TObject, std::size_t SIZE>
class ObjectPool
{
public:
    /**
     * Constructor.
     */
    ObjectPool() :
            m_freeList(nullptr)
    {
        for (std::size_t i = 0; i < m_objects.size(); ++i)
        {
            m_nodes[i].m_data = &m_objects[i];
            m_nodes[i].m_allocated = false;

            m_nodes[i].m_next = m_freeList;
            m_freeList = &m_nodes[i];
        }
    }

    /**
     * Destructor.
     */
    ~ObjectPool()
    {
        for (std::size_t i = 0; i < m_objects.size(); ++i)
        {
            assert(!m_nodes[i].m_allocated);
        }
    }

    /**
     * Allocates single object from pool.
     *
     * @return
     *      Allocated object or nullptr if there are no objects left in pool.
     */
    TObject* alloc()
    {
        if (m_freeList)
        {
            // get node
            auto node = m_freeList;

            // unlink
            m_freeList = m_freeList->m_next;
            node->m_next = nullptr;

            // mark as allocated
            assert(!node->m_allocated);
            node->m_allocated = true;

            // return object
            return node->m_data;
        }
        else
        {
            return nullptr;
        }
    }

    /**
     * Returns object to pool.
     *
     * @param object
     *      Object to release.
     */
    void release(TObject* object)
    {
        assert(object);

        // calculate index
        auto index = object - m_objects.data();

        // find node
        auto node = &m_nodes[index];

        // check if really allocated and points to right object
        assert(node->m_allocated);
        node->m_allocated = false;
        assert(node->m_data == object);
        assert(node->m_next == nullptr);

        node->m_next = m_freeList;
        m_freeList = node;
    }

private:
    /**
     * Pool node.
     */
    struct Node
    {
        /** Pointer to object. */
        TObject* m_data;
        /** Pointer to next free node. */
        Node* m_next;
        /** Flag indicating if object was allocated. */
        bool m_allocated;
    };

    /** Array with objects. */
    std::array<TObject, SIZE> m_objects;

    /** Array with nodes. */
    std::array<Node, SIZE> m_nodes;

    /** Number of currently used entries. */
    Node* m_freeList;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_OBJECTPOOL_HPP_*/
