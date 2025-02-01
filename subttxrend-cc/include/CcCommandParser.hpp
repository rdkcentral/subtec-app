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

#include "CcCommand.hpp"
#include "CcCommandProcessor.hpp"
#include "CcServiceBlock.hpp"
#include <subttxrend/common/Logger.hpp>

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <queue>
#include <memory>
#include <utility>

#define MAX_SERVICE_BLOCK_LENGTH 32

namespace subttxrend
{
namespace cc
{

class Clock
{
public:
    virtual std::chrono::time_point<std::chrono::steady_clock> now() = 0;
    virtual ~Clock() = default;
};

class SteadyClock : public Clock
{
public:
    std::chrono::time_point<std::chrono::steady_clock> now() override
    {
        return std::chrono::steady_clock::now();
    }
};

class CommandParser
{
public:
    explicit CommandParser(std::unique_ptr<Clock> clock = std::make_unique<SteadyClock>());
    void setProcessor(CommandProcessor *processor);
    void process(const ServiceBlock &block);

private:
    size_t handleC0(const uint8_t *data, size_t data_length);
    size_t handleC1(const uint8_t *data, size_t data_length);
    size_t handleC2(const uint8_t *data);
    size_t handleC3(const uint8_t *data);
    size_t handleG0(const uint8_t *data, size_t);
    size_t handleG1(const uint8_t *data, size_t);
    size_t handleG2G3(const uint8_t *data);
    size_t handleExtChar(const uint8_t *data, size_t);
    size_t handleC0P16(const uint8_t *data);

    void delay(std::chrono::milliseconds timeout);
    void delayCancel();
    void resetServiceBuffer();
    void processBlock(const ServiceBlock &block);
    bool handleDelay();

    struct RstOrDlc
    {
        int rst_pos = -1;
        int dlc_pos = -1;
    };
    static RstOrDlc scanForRSTDLC(const ServiceBlock &block);
    static PenAttributes decodePenAttributes(const uint8_t *data);
    static PenColor decodePenColor(const uint8_t *data);
    static WindowAttributes decodeWindowAttributes(const uint8_t *data);
    static WindowDefinition decodeWindowDefinition(uint8_t id, const uint8_t *data);
    static WindowAttributes getPredefinedWinAttr(uint8_t idx);
    static PenAttributes getPredefinedPenAttr(uint8_t idx);

private:
    const int DISPLAY_CAPTION_TIMEOUT = 10; // 10 secs
    common::Logger logger;
    CommandProcessor *m_proc;
    bool m_delayActive;
    std::chrono::time_point<std::chrono::steady_clock> m_timeout;
    static constexpr auto MAX_BLOCKS_IN_BUFFER = 128;
    std::queue<ServiceBlock> m_buffer;
    std::unique_ptr<Clock> m_clock;
    size_t m_blockSize;
    uint8_t m_serviceBlock[2 * MAX_SERVICE_BLOCK_LENGTH];
    size_t m_serviceBlockSize;
    bool m_cmdLengthExceeds;


    //
    // 608 caption processing
    //

public:
    void process608Data(uint32_t service, CcData::CcType ccType, uint8_t data1, uint8_t data2);
    void reset608();
    void set608();

private:
    static uint32_t m_608CaptionState;
    static uint32_t m_requested608Channel;
    static CcData::CcType m_608DataType;
    static uint32_t m_current608Channel;

    static uint8_t m_lastData1;
    static uint8_t m_lastData2;
    static bool m_isLastPairControl;
    static int m_lastBaseRow;

    std::vector<int> m_608Windowlist;

    uint32_t currentChannel(uint8_t data1, uint8_t data2);
    bool controlCode(uint8_t data1);
    bool xdsCode(uint8_t data1);
    bool textChannel(uint32_t channel);
    uint8_t miscControlCode(uint8_t data1, uint8_t data2);
    uint8_t fieldFromChannel(uint32_t channel);

    void reset608CaptionState();
    bool check608Parity(uint8_t dataByte);
    void insert608Char(uint8_t dataByte);
    void process608Char(uint8_t dataByte);
    void define608Windows();
    void define608Window(int window);
    void set608Background(uint8_t colourAttribute, uint32_t opacity, bool midrow);
    void set608StyleAttribute(uint8_t colourAttribute, bool underline, bool midrow);
    void set608Underline(bool underline);
    void special608Character(uint8_t code);
    void extended608Character(uint8_t c1, uint8_t c2);
    void process608TabOffset(uint8_t c2);
    void process608PAC(uint8_t c1, uint8_t c2);
    void process608control(uint8_t c1, uint8_t c2);
    void update608WindowDefinitionPosition(WindowDefinition &wd, uint32_t baseRow, uint32_t indent);
    void set608WindowPosition(int nState, unsigned char nBaseRow, unsigned char nCol);
    void set608WindowCursor(uint32_t row, uint32_t col);
    void set608WindowCursor(uint32_t row);
    void set608WindowFill(uint32_t colour, uint32_t opacity);
    void clear608Windows(void);
    void erase608NonDisplayedMemory(void);
    void erase608DisplayedMemory(void);
    void set608timeout(int timeout);
    void process608CaptionControl(uint8_t c1, uint8_t control);
    void process608ResumeCaptionLoading();
    void process608Backspace();
    void process608AlarmOn();
    void process608AlarmOff();
    void process608DeleteToEndOfRow();
    void process608Rollup(uint8_t control);
    void process608FlashOn();
    void process608ResumeDirectCaptioning();
    void process608TextRestart();
    void process608ResumeTextDisplay();
    void process608Erase608DisplayedMemory();
    void process608CariageReturn();
    void process608Erase608NonDisplayedMemory();
    void process608EndOfCaption();
    int32_t get608WindowFromRow(int32_t row);
    WindowsMap windowIdTo608WindowMap(int windowId);
    void removeFrom608WindowList(int window);
    int active608Window();
    void clear608Window(uint32_t window);
};

} // namespace cc
} // namespace subttxrend
