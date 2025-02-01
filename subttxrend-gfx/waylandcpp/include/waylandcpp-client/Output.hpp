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


#ifndef WAYLANDCPP_OUTPUT_HPP_
#define WAYLANDCPP_OUTPUT_HPP_

#include "Types.hpp"

#include <string>

namespace waylandcpp
{

class OutputListenerWrapper;

/**
 * Listener for output events.
 */
class OutputListener
{
public:
    /** Current mode flag. */
    static const uint32_t OUTPUT_MODE_FLAG_CURRENT;

    /** Preferred mode flag. */
    static const uint32_t OUTPUT_MODE_FLAG_PREFERRED;

    /**
     * Constructor.
     */
    OutputListener() = default;

    /**
     * Destructor.
     */
    virtual ~OutputListener() = default;

    /**
     * Properties of the output
     *
     * @param object
     *      Object for which event is called.
     * @param x
     *      X position within the global compositor space
     * @param y
     *      Y position within the global compositor space
     * @param physicalWidth
     *      Width in millimeters of the output
     * @param physicalHeight
     *      Height in millimeters of the output
     * @param subpixel
     *      Subpixel orientation of the output
     * @param make
     *      Textual description of the manufacturer
     * @param model
     *      Textual description of the model
     * @param transform
     *      Transform that maps framebuffer to output
     */
    virtual void geometry(OutputPtr object,
                          int32_t x,
                          int32_t y,
                          int32_t physicalWidth,
                          int32_t physicalHeight,
                          int32_t subpixel,
                          const char *make,
                          const char *model,
                          int32_t transform) = 0;

    /**
     * advertise available modes for the output
     *
     * @param object
     *      Object for which event is called.
     * @param flags
     *      Bitfield of mode flags
     * @param width
     *      Width of the mode in hardware units
     * @param height
     *      Height of the mode in hardware units
     * @param refresh
     *      Vertical refresh rate in mHz
     */
    virtual void mode(OutputPtr object,
                      uint32_t flags,
                      int32_t width,
                      int32_t height,
                      int32_t refresh) = 0;

    /**
     * sent all information about output
     *
     * @param object
     *      Object for which event is called.
     */
    virtual void outputEventsDone(OutputPtr object) = 0;

    /**
     * output scaling properties
     *
     * @param object
     *      Object for which event is called.
     * @param factor
     *      Scaling factor of output
     */
    virtual void scale(OutputPtr object,
                       int32_t factor) = 0;
};

/**
 * Output (common).
 */
class Output : public Proxy<Output, wl_output>
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Output> Ptr;

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
    virtual ~Output();

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
    bool setListener(OutputListener* listener);

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
    Output(WaylandObjectType* const wlObject,
           DestructorFunc const destructorFunction);

private:
    /** Wrapper for object listeners. */
    std::unique_ptr<OutputListenerWrapper> m_listenerWrapper;
};

/**
 * Output (version 1).
 */
class Output1 : public Output
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Output1> Ptr;

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
    Output1(WaylandObjectType* const wlObject);

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
    Output1(WaylandObjectType* const wlObject,
            DestructorFunc const destructorFunction) :
            Output(wlObject, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_OUTPUT_HPP_*/
