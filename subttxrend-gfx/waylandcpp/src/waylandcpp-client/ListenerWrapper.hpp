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


#ifndef WAYLANDCPP_LISTENERWRAPPER_HPP_
#define WAYLANDCPP_LISTENERWRAPPER_HPP_

#include "Types.hpp"

namespace waylandcpp
{

/**
 * Wrapper over object listeners - base class.
 *
 * @tparam ObjectType
 *      Listener owner type.
 * @tparam ListenerType
 *      Listener type.
 * @tparam WaylandObjectType
 *      Native wayland - listener owner type.
 * @tparam WaylandListenerType
 *      Native wayland - listener type.
 */
template<class ObjectType, class ListenerType, class WaylandObjectType,
        class WaylandListenerType>
class ListenerWrapper
{
public:
    /** Weak pointer to owner. */
    typedef std::shared_ptr<ObjectType> ObjectPtr;

    /** Weak pointer to owner. */
    typedef std::weak_ptr<ObjectType> WeakObjectPtr;

    /**
     * Add listener function type.
     *
     * @param wlObject
     *      Object to which listener is added.
     * @param wlListener
     *      Listener to add.
     * @param data
     *      User defined data.
     *
     * @return
     *      0 on success, -1 on error.
     */
    typedef int (*AddListenerFunc)(WaylandObjectType* wlObject,
                                   const WaylandListenerType* wlListener,
                                   void* data);

    /**
     * Constructor.
     *
     * @param owner
     *      Object owning the wrapper.
     * @param addFunction
     *      Native function to add listener.
     */
    ListenerWrapper(WeakObjectPtr owner,
                    AddListenerFunc addFunction) :
            m_owner(owner),
            m_addFunction(addFunction),
            m_wlListener(),
            m_listener(nullptr)
    {
        // noop
    }

    /**
     * Sets (adds) listener.
     *
     * @param wlObject
     *      Native object to which listener is added.
     * @param listener
     *      Listener to add.
     *
     * @return
     *      True on success, false on error.
     */
    bool setListener(WaylandObjectType* wlObject,
                     ListenerType* listener)
    {
        if ((*m_addFunction)(wlObject, &m_wlListener, this) == 0)
        {
            m_listener = listener;
            return true;
        }
        else
        {
            return false;
        }
    }

protected:
    /**
     * Returns native listener object.
     *
     * It is expected subclasses will fill the callbacks in this object.
     *
     * @return
     *      Reference to native listener object.
     */
    WaylandListenerType& getListenerStruct()
    {
        return m_wlListener;
    }

    /**
     * Returns owner of the listener.
     *
     * @param data
     *      User defined data (from listener callback).
     *
     * @return
     *      Owner of the listener.
     */
    static ObjectPtr getOwner(void* data)
    {
        ListenerWrapper* thiz = reinterpret_cast<ListenerWrapper*>(data);
        return thiz->m_owner.lock();
    }

    /**
     * Returns the listener.
     *
     * @param data
     *      User defined data (from listener callback).
     *
     * @return
     *      Listener object.
     */
    static ListenerType* getListener(void* data)
    {
        ListenerWrapper* thiz = reinterpret_cast<ListenerWrapper*>(data);
        return thiz->m_listener;
    }

private:
    /** Object owning the wrapper (and listener). */
    WeakObjectPtr m_owner;

    /** Native function to add listener. */
    AddListenerFunc m_addFunction;

    /** Native listener object. */
    WaylandListenerType m_wlListener;

    /** Listener object set. */
    ListenerType* m_listener;
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_LISTENERWRAPPER_HPP_*/
