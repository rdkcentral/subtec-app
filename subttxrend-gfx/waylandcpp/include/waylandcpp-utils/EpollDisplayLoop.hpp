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


#ifndef WAYLANDCPP_EPOLLDISPLAYLOOP_HPP_
#define WAYLANDCPP_EPOLLDISPLAYLOOP_HPP_

#include <atomic>
#include <memory>
#include <thread>
#include <mutex>

#include "Epoll.hpp"
#include "EpollDisplayHandler.hpp"
#include "EpollEventHandler.hpp"

namespace waylandcpp
{

class EpollDisplayLoop;

/**
 * Listener for display loop events.
 */
class EpollDisplayLoopListener
{
public:
    /**
     * Constructor.
     */
    EpollDisplayLoopListener() = default;

    /**
     * Destructor.
     */
    virtual ~EpollDisplayLoopListener() = default;

    /**
     * Display events loop started.
     *
     * @param loop
     *      Loop that called the notification.
     */
    virtual void started(EpollDisplayLoop* loop) = 0;

    /**
     * Display events loop finished.
     *
     * @param loop
     *      Loop that called the notification.
     */
    virtual void finished(EpollDisplayLoop* loop) = 0;

    /**
     * Wakup event received.
     *
     * @param loop
     *      Loop that called the notification.
     */
    virtual void wakeupReceived(EpollDisplayLoop* loop) = 0;
};

/**
 * Epoll based wayland display loop.
 */
class EpollDisplayLoop : private EpollEventHandlerListener
{
public:
    /**
     * Constructor.
     */
    EpollDisplayLoop();

    /**
     * Destructor.
     */
    virtual ~EpollDisplayLoop();

    /**
     * Starts the loop.
     *
     * @param display
     *      Display for which loop is started.
     * @param listener
     *      Listener to call with events.
     *
     * @return
     *      True on success, false on error.
     */
    bool start(DisplayPtr display,
               EpollDisplayLoopListener* listener);

    /**
     * Sends wakup event.
     */
    void wakeup();

    /**
     * Requests loop termination.
     */
    void stop();

private:
    /** Mutex type. */
    typedef std::mutex Mutex;

    /** Mutex guard type. */
    typedef std::lock_guard<Mutex> MutexGuard;

    /** EpollEventHandlerListener::wakeupReceived */
    virtual void wakeupReceived() override;

    /**
     * Thread loop.
     *
     * This is the main function running the display loop.
     */
    void threadLoop();

    /**
     * Method to called by the loop.
     *
     * @param thiz
     *      Display looop pointer.
     */
    static void staticThreadLoop(EpollDisplayLoop* thiz);

    /** Mutex used for synchronization. */
    Mutex m_mutex;

    /** Display main loop thread. */
    std::unique_ptr<std::thread> m_thread;

    /** Epool to use. */
    std::unique_ptr<Epoll> m_epoll;

    /** Display handler to use. */
    std::unique_ptr<EpollDisplayHandler> m_displayHandler;

    /** Event handler to use. */
    std::unique_ptr<EpollEventHandler> m_eventHandler;

    /** Display for which loop was starter. */
    DisplayPtr m_display;

    /** Loop running flag. */
    std::atomic<bool> m_running;

    /** Loop stop requested flag. */
    std::atomic<bool> m_stopRequested;

    /** Listeners to call with events. */
    EpollDisplayLoopListener* m_listener;
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_EPOLLDISPLAYLOOP_HPP_*/
