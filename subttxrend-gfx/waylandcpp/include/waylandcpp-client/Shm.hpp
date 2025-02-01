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


#ifndef WAYLANDCPP_SHM_HPP_
#define WAYLANDCPP_SHM_HPP_

#include "Types.hpp"

#include <string>

#include "ShmPool.hpp"

namespace waylandcpp
{

class ShmListenerWrapper;

/**
 * Listener for SHM events.
 */
class ShmListener
{
public:
    /**
     * Constructor.
     */
    ShmListener() = default;

    /**
     * Destructor.
     */
    virtual ~ShmListener() = default;

    /**
     * pixel format description
     *
     * @param object
     *      Object for which event is called.
     * @param nativeFormat
     *      Buffer pixel format
     */
    virtual void format(ShmPtr object,
                        uint32_t nativeFormat) = 0;
};

/**
 * Shm (common).
 */
class Shm : public Proxy<Shm, wl_shm>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Shm> Ptr;

    static const wl_interface* getWlInterface();

    /**
     * Destructor.
     */
    virtual ~Shm();

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
    bool setListener(ShmListener* listener);

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
    Shm(WaylandObjectType* const wlObject,
        DestructorFunc const destructorFunction);

private:
    /** Wrapper for object listeners. */
    std::unique_ptr<ShmListenerWrapper> m_listenerWrapper;
};

/**
 * Shm (version 1).
 */
class Shm1 : public Shm
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Shm1> Ptr;

    /** Minimum required object version. */
    static const uint32_t OBJECT_VERSION = 1;

    /**
     * Constructor.
     *
     * Uses default destructor function.
     *
     * @param wlObject
     *      Wrapped wayland object.
     */
    Shm1(WaylandObjectType* const wlObject);

    /**
     * Creates pool.
     *
     * @param file
     *      File to be used for buffering.
     *
     * @return
     *      Created pool or null on error.
     */
    ShmPoolPtr createPool(FilePtr file);

    /**
     * Creates pool.
     *
     * @param file
     *      File to be used for buffering.
     *
     * @return
     *      Created pool or null on error.
     *
     * @tparam ShmPoolInterface
     *      Requested pool interface.
     */
    template<class ShmPoolInterface>
    typename ShmPoolInterface::Ptr createPool(FilePtr file)
    {
        ShmPoolPtr ptr = createPool(file);

        if (!ptr)
        {
            return nullptr;
        }

        return ptr->getInterface<ShmPoolInterface>();
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
    Shm1(WaylandObjectType* const wlObject,
         DestructorFunc const destructorFunction) :
            Shm(wlObject, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_SHM_HPP_*/
