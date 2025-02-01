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


#ifndef WAYLANDCPP_SHMPOOL_HPP_
#define WAYLANDCPP_SHMPOOL_HPP_

#include "Types.hpp"

#include <string>

#include "Buffer.hpp"

namespace waylandcpp
{

/**
 * Shm Pool (common).
 */
class ShmPool : public Proxy<ShmPool, wl_shm_pool>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<ShmPool> Ptr;

    /**
     * Returns object wayland interface.
     *
     * @return
     *      Pointer to wayland interface.
     */
    static const wl_interface* getWlInterface();

    /**
     * Destructor.
     */
    virtual ~ShmPool();

    /** @copydoc Proxy::setUserData */
    virtual void setUserData(void* userData) override final;

    /** @copydoc Proxy::getUserData */
    virtual void* getUserData() const override final;

    /**
     * Returns pool size.
     */
    std::size_t getSize() const;

protected:
    /**
     * Constructor.
     *
     * For derived types.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param file
     *      File used as a buffer.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    ShmPool(WaylandObjectType* const wlObject,
            FilePtr file,
            DestructorFunc const destructorFunction);

    /** File used as a buffer. */
    FilePtr m_file;
};

/**
 * Shm Pool (version 1).
 */
class ShmPool1 : public ShmPool
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<ShmPool1> Ptr;

    /** Minimum required object version. */
    static const uint32_t OBJECT_VERSION = 1;

    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param file
     *      File used as a buffer.
     */
    ShmPool1(WaylandObjectType* const wlObject,
             FilePtr file);

    /**
     * Creates buffer with given params.
     *
     * @param params
     *      Buffer parameters.
     *
     * @return
     *      Created buffer or null on error.
     */
    BufferPtr createBuffer(const BufferParams& params);

    /**
     * Creates buffer with given params.
     *
     * @param params
     *      Buffer parameters.
     *
     * @return
     *      Created buffer or null on error.
     *
     * @tparam BufferInterface
     *      Requested buffer interface.
     */
    template<class BufferInterface>
    typename BufferInterface::Ptr createBuffer(const BufferParams& params)
    {
        BufferPtr ptr = createBuffer(params);

        if (!ptr)
        {
            return nullptr;
        }

        return ptr->getInterface<BufferInterface>();
    }

protected:
    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param file
     *      File used as a buffer.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    ShmPool1(WaylandObjectType* const wlObject,
             FilePtr file,
             DestructorFunc const destructorFunction) :
            ShmPool(wlObject, file, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_SHMPOOL_HPP_*/
