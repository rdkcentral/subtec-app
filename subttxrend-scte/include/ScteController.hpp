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

#include <cinttypes>
#include <queue>

#include <subttxrend/common/StcProvider.hpp>
#include <subttxrend/protocol/PacketData.hpp>
#include "ScteRenderer.hpp"
#include "ScteTable.hpp"
#include "ScteSectionAssembler.hpp"

namespace subttxrend
{
namespace scte
{

class Controller
{
public:
    Controller();
    virtual ~Controller();

    bool init(gfx::Window* gfxWindow, common::StcProvider* stcProvider);

    virtual void process();

    virtual bool addData(const protocol::PacketData& dataPacket);

    virtual bool start();

    virtual void shutdown();

    virtual bool stop();

    virtual bool isStarted() const;

    virtual void mute();

    virtual void unmute();

    virtual bool isMuted() const;

private:
    bool m_isStarted;
    bool m_isMuted;
    static constexpr std::uint32_t PCR_SEC = 90000;
    static constexpr std::uint32_t DISPLAY_TIME_OFFSET_PCR = PCR_SEC / 20; // 50ms

    common::StcProvider* m_stcProvider;
    SectionAssembler m_decoder;
    std::queue<ScteTablePtr> m_database;
    std::unique_ptr<Renderer> m_renderer;
    ScteTablePtr m_presented;

    bool isTimeToDisplay(const ScteTablePtr& table, std::uint32_t stc);
    void processPackets(std::uint32_t stc);
    void render(const ScteTable* table);
    void clearDatabase();
};

} // namespace scte
} // namespace subttxrend

