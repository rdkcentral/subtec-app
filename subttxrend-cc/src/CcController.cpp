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


#include "CcController.hpp"
#include "CcServiceBlock.hpp"
#include "CcRenderer.hpp"

#include <subttxrend/common/Logger.hpp>

#include <stdexcept>
#include <cmath>
#include <memory>


namespace subttxrend
{
namespace cc
{

Controller::Controller()
    :   started(false),
        muted(true),
        activeType(CeaType::CEA_608),
        activeService(0),
        incompCcp(nullptr),
        logger("ClosedCaptions", "CcController")
{
    logger.info("%s", __func__);
}

Controller::~Controller()
{
    delete incompCcp;
}

bool Controller::init(gfx::Window* gfxWindow, std::shared_ptr<gfx::PrerenderedFontCache> fontCache)
{
    logger.info("%s", __func__);

    renderer.reset(new Renderer(gfxWindow));
    m_fontCache = fontCache;
    winCtrl = std::make_unique<WindowController>(renderer, m_fontCache);
    parser.setProcessor(winCtrl.get());

    logger.info("%s - done", __func__);

    return true;
}

void Controller::shutdown()
{
    logger.info("%s", __func__);

    stop();

    logger.info("%s - done", __func__);
}

bool Controller::addData(const protocol::PacketData& dataPacket)
{
    logger.trace("%s", __func__);

    if (!started)
    {
        logger.error("%s - skipping, not started", __func__);
        return false;
    }

    UserData* cc = new UserData((const std::uint8_t* ) dataPacket.getData(),
                                dataPacket.getDataSize());
    cc708Data.emplace_back(std::move(cc));

    logger.trace("%s - done", __func__);

    return true;
}

void Controller::setActiveService(CeaType type, std::uint32_t serviceNo)
{
    logger.info("%s: type %d, service %d", __func__, (int) type, (int) serviceNo);

    if((type != activeType) || (serviceNo != activeService))
    {
        activeType = type;
        activeService = serviceNo;

        stop();

        // TODO: clean graphics
    }

    logger.info("%s - done", __func__);
}

bool Controller::start()
{
    logger.info("%s", __func__);

    if(!started)
    {
        if(activeType == CeaType::CEA_608)
        {
            parser.set608();
        }

        started = true;
        renderer->clear();
        renderer->update();
        if(!muted)
        {
            renderer->show();
        }
    }

    logger.info("%s - done", __func__);

    return true;
}

bool Controller::stop()
{
    logger.info("%s", __func__);

    if(started)
    {
        started = false;
        parser.reset608();
        winCtrl->reset();
        renderer->clear();
        renderer->update();
        renderer->hide();
        purgeQueues();
    }

    logger.info("%s - done", __func__);

    return true;
}

bool Controller::isStarted() const
{
    return started;
}

void Controller::mute()
{
    logger.info("%s", __func__);

    if (!muted)
    {
        muted = true;

        if (started)
        {
            renderer->hide();
        }
    }

    logger.info("%s - done", __func__);
}

void Controller::unmute()
{
    logger.info("%s", __func__);

    if (muted)
    {
        muted = false;

        if (started)
        {
            renderer->show();
        }
    }

    logger.info("%s - done", __func__);
}

bool Controller::isMuted() const
{
    return muted;
}

void Controller::process()
{
    logger.trace("%s", __func__);

    if (!started || muted)
    {
        logger.trace("%s - started %d, muted %d", __func__, started, muted);
	if (!cc708Data.empty())
	{
		cc708Data.clear();
	}
	if (!cc708Ccp.empty())
	{
		cc708Ccp.clear();
	}
	if (!cc708Sb.empty())
	{
		cc708Sb.clear();
	}
        return;
    }

    processCcDataQueue();
    processCcpQueue();
    processSbQueue();
    winCtrl->drawWindows();

    logger.trace("%s - done", __func__);
}

void Controller::purgeQueues()
{
    cc708Data.clear();
    cc708Ccp.clear();
    cc708Sb.clear();
}

void Controller::processCcDataQueue()
{
    logger.trace("%s CC pairs %zu", __func__, cc708Data.size());

    for(auto& userData : cc708Data)
    {
        if(userData->isValid())
        {
            for(auto& ccData : userData->getCcData())
            {
                if(activeType == CeaType::CEA_708)
                {
                    if(ccData->isCcpStart())
                    {
                        logger.debug("CCP start");

                        putCcpToQueue();

                        try
                        {
                            incompCcp = new CaptionChannelPacket(*ccData);
                        }
                        catch(std::exception& e)
                        {
                            // Incorrect data - skip it
                            logger.error("%s", e.what());
                            continue;
                        }
                    }
                    else if(ccData->isCcpData())
                    {
                        logger.debug("CCP data");

                        if(incompCcp != nullptr)
                        {
                            try
                            {
                                incompCcp->addCcData(*ccData);
                            }
                            catch(std::exception& e)
                            {
                                // Incorrect data - skip it
                                logger.error("%s", e.what());
                                continue;
                            }

                            if(incompCcp->isFull())
                            {
                                logger.debug("CCP complete");

                                putCcpToQueue();
                            }
                        }
                    }
                }
                else if(activeType == CeaType::CEA_608)
                {
                    if(ccData->is608Data())
                    {
                        logger.debug("608 data");
                        if (ccData->ccValid)
                        {
                            parser.process608Data(activeService, ccData->ccType, ccData->data1, ccData->data2);
                        }
                    }
                }

                if(ccData->isPadding())
                {
                    logger.debug("CC Padding - skipping");
                }
            }
        }
    }

    cc708Data.clear();
}

void Controller::processCcpQueue()
{
    std::size_t offset;

    logger.trace("%s CCP packets %zu", __func__, cc708Ccp.size());

    for(auto& ccp : cc708Ccp)
    {
        offset = 0;
        auto ccpData = ccp->getCcpData();
        while (offset < ccpData.size())
        {
            std::unique_ptr<ServiceBlock> sb(new ServiceBlock(ccpData, offset));

            if (sb->getBlockSize() == 0) {
                // [CEA-708-E 6.2.3]
                logger.trace("Null Service Block. Ignoring and moving to the next CCP.");
                break;
            }

            if(activeService == sb->getServiceNumber())
            {
                // TODO: Check sequence number correctness
                cc708Sb.emplace_back(std::move(sb));
            }
        }
    }

    cc708Ccp.clear();
}

void Controller::processSbQueue()
{
    for(auto& sb : cc708Sb)
    {
        parser.process(*sb);
    }

    cc708Sb.clear();
}

void Controller::putCcpToQueue()
{
    if(incompCcp != nullptr)
    {
        cc708Ccp.emplace_back(std::move(incompCcp));
        incompCcp = nullptr;
    }
}

void Controller::processSetCCAttributesPacket(const protocol::PacketSetCCAttributes& packet)
{
    winCtrl->processSetCCAttributesPacket(packet);
}

void Controller::setTextForPreview(const std::string& text)
{
    logger.info("%s", __func__);
    // We're using window 5 (just because) for the preview
    WindowsMap wm = toWindowsMap(255 /*1<<(5-1)*/);
    logger.trace("deleteWindows");
    winCtrl->deleteWindows(wm);
    WindowDefinition wd{5, 1, false, false, true, false, 0, 0, 1, {}, 32, {}, {}};
    logger.trace("defineWindow");
    winCtrl->defineWindow(wd);
    PenAttributes pa;
    logger.trace("setPenAttributes");
    winCtrl->setPenAttributes(pa);
    PenColor pc;
    logger.trace("setPenColor + setPenLocation");
    winCtrl->setPenColor(pc);
    winCtrl->setPenLocation(0, 0);
    logger.trace("report");
    winCtrl->report(text);
    logger.trace("displayWindows");
    winCtrl->displayWindows(wm);
    logger.trace("drawWindows");
    winCtrl->drawWindows();
    logger.info("%s ends", __func__);
}

} // namespace cc
} // namespace subttxrend

