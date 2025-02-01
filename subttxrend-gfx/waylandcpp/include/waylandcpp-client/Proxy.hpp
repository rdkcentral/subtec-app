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


#ifndef WAYLANDCPP_PROXY_HPP_
#define WAYLANDCPP_PROXY_HPP_

#include <memory>

namespace waylandcpp
{

/**
 * Proxied Wayland objects base class.
 */
template<class TDerivedType, class TWaylandObjectType>
class Proxy : public std::enable_shared_from_this<
        Proxy<TDerivedType, TWaylandObjectType>>
{
    Proxy(const Proxy&) = delete;
    Proxy& operator=(const Proxy&) = delete;

    /** Shared class base type. */
    typedef std::enable_shared_from_this<Proxy<TDerivedType, TWaylandObjectType>> SharedBase;

public:
    /** Type of derived object. */
    typedef TDerivedType DerivedType;

    /** Type of wrapped object. */
    typedef TWaylandObjectType WaylandObjectType;

    /**
     * Destructor function type.
     *
     * @param wlObject
     *      Object on which operation is executed.
     */
    typedef void (*DestructorFunc)(WaylandObjectType* wlObject);

    /**
     * Constructor.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    Proxy(WaylandObjectType* const wlObject,
          DestructorFunc const destructorFunction) :
            m_wlObject(wlObject),
            m_destructorFunction(destructorFunction)
    {
        // noop
    }

    /**
     * Destructor.
     */
    virtual ~Proxy()
    {
        (*m_destructorFunction)(m_wlObject);
    }

    /**
     * Returns wrapped native object.
     *
     * @return
     *      Wrapped native object.
     */
    WaylandObjectType* getNativeObject() const
    {
        return m_wlObject;
    }

    /**
     * Sets user data.
     *
     * @param userData
     *      User data to associated with the object.
     */
    virtual void setUserData(void* userData) = 0;

    /**
     * Returns user data.
     *
     * @return
     *      User data set with setUserData().
     */
    virtual void* getUserData() const = 0;

    /**
     * Returns specific version of the interface.
     *
     * @return
     *      Pointer to interface if supported, nullptr otherwise.
     *
     * @tparam TargetType
     *      Requested version of the interface.
     */
    template<class TargetType>
    std::shared_ptr<TargetType> getInterface()
    {
        auto sourceShared = makeShared();

        return std::static_pointer_cast < TargetType > (sourceShared);
    }

protected:
    /**
     * Makes shared pointer for this object.
     *
     * @return
     *      Created pointer.
     */
    std::shared_ptr<DerivedType> makeShared()
    {
        auto thisShared = SharedBase::shared_from_this();

        return std::static_pointer_cast < DerivedType > (thisShared);
    }

private:
    /** Wrapped wayland object. */
    WaylandObjectType* const m_wlObject;

    /** Sets of callbacks to use. */
    DestructorFunc const m_destructorFunction;
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_PROXY_HPP_*/
