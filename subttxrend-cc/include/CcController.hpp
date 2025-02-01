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
*
* Copyright 2023 Comcast Cable Communications Management, LLC
* Licensed under the Apache License, Version 2.0
*****************************************************************************/

#pragma once
#include <cinttypes>
#include <vector>
#include <subttxrend/protocol/PacketData.hpp>
#include <subttxrend/protocol/PacketSetCCAttributes.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/common/Logger.hpp>
#include "CcUserData.hpp"
#include "CcCaptionChannelPacket.hpp"
#include "CcServiceBlock.hpp"
#include "CcWindowController.hpp"
#include "CcCommandParser.hpp"

namespace subttxrend
{
namespace cc
{

enum class CeaType
{
    CEA_608,
    CEA_708
};

class Controller
{
public:
    Controller();
    virtual ~Controller();

    bool init(gfx::Window* gfxWindow, std::shared_ptr<gfx::PrerenderedFontCache> fontCache);

    virtual void process();

    virtual bool addData(const protocol::PacketData& dataPacket);

    virtual void setActiveService(CeaType type, std::uint32_t serviceNo);

    virtual bool start();

    virtual void shutdown();

    virtual bool stop();

    virtual bool isStarted() const;

    virtual void mute();

    virtual void unmute();

    virtual bool isMuted() const;

    virtual void processSetCCAttributesPacket(const protocol::PacketSetCCAttributes& packet);


    void setTextForPreview(const std::string& text);

private:
    void purgeQueues();
    void processCcDataQueue();
    void processCcpQueue();
    void processSbQueue();
    void putCcpToQueue();

    bool                                started;
    bool                                muted;

    CeaType                             activeType;
    std::uint32_t                       activeService;

    CaptionChannelPacket*               incompCcp;

    template<typename T>
    using PtrVec = std::vector<std::unique_ptr<T>>;
    PtrVec<UserData>                    cc708Data;
    PtrVec<CaptionChannelPacket>        cc708Ccp;
    PtrVec<ServiceBlock>                cc708Sb;

    common::Logger                      logger;
    CommandParser parser;
    std::unique_ptr<WindowController>   winCtrl;
    std::shared_ptr<Gfx> renderer;
    std::shared_ptr<gfx::PrerenderedFontCache> m_fontCache;
};

} // namespace cc
} // namespace subttxrend

