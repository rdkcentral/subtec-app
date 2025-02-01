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


#ifndef WAYLANDCPP_REGISTRY_HPP_
#define WAYLANDCPP_REGISTRY_HPP_

#include "Types.hpp"
#include "ObjectFactory.hpp"

#include <string>

namespace waylandcpp
{

class RegistryListenerWrapper;

/**
 * Listener for registry events.
 */
class RegistryListener
{
public:
    /**
     * Constructor.
     */
    RegistryListener() = default;

    /**
     * Destructor.
     */
    virtual ~RegistryListener() = default;

    /**
     * Announce global object.
     *
     * @param object
     *      Object for which event is called.
     * @param name
     *      Numeric name of the global object.
     * @param interface
     *      Name of the interface implemented by the object.
     * @param version
     *      Interface version.
     */
    virtual void global(RegistryPtr object,
                        uint32_t name,
                        std::string interface,
                        uint32_t version) = 0;

    /**
     * Announce removal of global object.
     *
     * @param object
     *      Object for which event is called.
     * @param name
     *      Numeric name of the global object.
     */
    virtual void globalRemove(RegistryPtr object,
                              uint32_t name) = 0;
};

/**
 * Registry (common).
 */
class Registry : public Proxy<Registry, wl_registry>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Registry> Ptr;

    /**
     * Destructor.
     */
    virtual ~Registry();

    /** @copydoc Proxy::setUserData */
    virtual void setUserData(void* userData) override final;

    /** @copydoc Proxy::getUserData */
    virtual void* getUserData() const override final;

    /**
     * Sets object listner.
     *
     * @param listener
     *      Listner to set.
     *
     * @retval true
     *      Success.
     * @retval false
     *      Failure (e.g. listener already set).
     */
    bool setListener(RegistryListener* listener);

protected:
    /**
     * Constructor.
     *
     * For derived types.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    Registry(WaylandObjectType* const wlObject,
             DestructorFunc const destructorFunction);

private:
    /** Wrapper for object listeners. */
    std::unique_ptr<RegistryListenerWrapper> m_listenerWrapper;
};

/**
 * Registry (version 1).
 */
class Registry1 : public Registry
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Registry1> Ptr;

    /** Minimum required object version. */
    static const uint32_t OBJECT_VERSION = 0;

    /**
     * Constructor.
     *
     * Uses default destructor function.
     *
     * @param wlObject
     *      Wrapped wayland object.
     */
    Registry1(WaylandObjectType* const wlObject);

    /**
     * Binds the interface.
     *
     * @param name
     *      Name (identifier) of the interface to bind.
     *
     * @return
     *      Interface bound or null on error.
     *
     * @tparam InterfaceType
     *      Requested interface to bind.
     */
    template<class InterfaceType>
    typename InterfaceType::Ptr bind(uint32_t name)
    {
        typedef typename InterfaceType::WaylandObjectType IfaceWaylandObjectType;

        auto wlInterface = InterfaceType::getWlInterface();
        if (!wlInterface)
        {
            return nullptr;
        }

        void* nativePtr = nativeBind(name, wlInterface,
                InterfaceType::OBJECT_VERSION);
        if (!nativePtr)
        {
            return nullptr;
        }

        IfaceWaylandObjectType* waylandPtr =
                reinterpret_cast<IfaceWaylandObjectType*>(nativePtr);

        auto typePtr = ObjectFactory::create(waylandPtr);
        if (!typePtr)
        {
            return nullptr;
        }

        return typePtr->template getInterface<InterfaceType>();
    }

protected:
    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    Registry1(WaylandObjectType* const wlObject,
              DestructorFunc const destructorFunction) :
            Registry(wlObject, destructorFunction)
    {
        // noop
    }

private:
    /**
     * Performs native bind operation.
     *
     * @param name
     *      Name (identifier) of the interface.
     * @param interface
     *      Pointer to native wayland interface.
     * @param version
     *      Requested version.
     */
    void* nativeBind(uint32_t name,
                     const wl_interface* interface,
                     uint32_t version);
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_REGISTRY_HPP_*/
