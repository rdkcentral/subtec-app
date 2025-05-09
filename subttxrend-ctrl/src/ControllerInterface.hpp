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


#pragma once

#include <chrono>
#include <mutex>

#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/protocol/Packet.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/protocol/PacketMute.hpp>
#include <subttxrend/protocol/PacketSetCCAttributes.hpp>
#include <subttxrend/common/ConfigProvider.hpp>

namespace subttxrend {
namespace ctrl {

class ControllerInterface : private common::NonCopyable
{
  public:
    virtual ~ControllerInterface() = default;

    /**
     * Performs data processing.
     */
    virtual void process() = 0;
    virtual void addData(const protocol::PacketData& dataPacket) = 0;

    /**
     * Returns time to wait before next call to process required.
     *
     * @return
     *      Time to wait before next call to process. std::chrono::milliseconds::zero()
     *      for unknown time - when there is no data queued or in paused state.
     */
    virtual std::chrono::milliseconds getWaitTime() const { return std::chrono::milliseconds(50);};

    virtual void activate() = 0;
    virtual void deactivate() = 0;

    virtual void mute(bool mute) = 0;
    virtual bool wantsData(protocol::PacketChannelSpecific const& packet) const = 0;

    virtual void processTimestamp(const protocol::Packet& ){}
    virtual void processInfo(const protocol::Packet& ){}
    virtual void flush() {}
    virtual void pause() {}
    virtual void resume() {}
    virtual void processSetCCAttributesPacket(const protocol::PacketSetCCAttributes&){}
};

template<class Ctrl>
class MutexedController final : public ControllerInterface
{
  public:

    template <class... Ts>
    MutexedController(Ts&&... args)
        : controller(std::forward<Ts>(args)...)
    {
    }

    void process() override
    {
        std::lock_guard<std::mutex> lock{mutex};
        controller.process();
    }

    void addData(const protocol::PacketData& dataPacket) override
    {
        std::lock_guard<std::mutex> lock{mutex};
        controller.addData(dataPacket);
    }

    virtual void activate() override
    {
        std::lock_guard<std::mutex> lock{mutex};
        controller.activate();
    }
    virtual void deactivate() override
    {
        std::lock_guard<std::mutex> lock{mutex};
        controller.deactivate();
    }

    void mute(bool m) override
    {
        std::lock_guard<std::mutex> lock{mutex};
        controller.mute(m);
    }

    bool wantsData(protocol::PacketChannelSpecific const& packet) const override
    {
        std::lock_guard<std::mutex> lock{mutex};
        return controller.wantsData(packet);
    }

    void processTimestamp(const protocol::Packet& packet) override
    {
        std::lock_guard<std::mutex> lock{mutex};
        controller.processTimestamp(packet);
    }

    void flush() override
    {
        std::lock_guard<std::mutex> lock{mutex};
        controller.flush();
    }

    void pause() override
    {
        std::lock_guard<std::mutex> lock{mutex};
        controller.pause();
    }

    void resume() override
    {
        std::lock_guard<std::mutex> lock{mutex};
        controller.resume();
    }
    void processSetCCAttributesPacket(const protocol::PacketSetCCAttributes& packet) override
    {
        std::lock_guard<std::mutex> lock{mutex};
        controller.processSetCCAttributesPacket(packet);
    }

  private:
    Ctrl controller;
    std::mutex mutable mutex;
};
} /* namespace ctrl */
} /* namespace subttxrend */
