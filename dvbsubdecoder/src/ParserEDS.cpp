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


#include "ParserEDS.hpp"

#include <subttxrend/common/Logger.hpp>

#include "PesPacketReader.hpp"
#include "Database.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "ParserEDS");

} // namespace <anonmymous>

void ParserEDS::parseEndOfDisplaySetSegment(Database& database,
                                            PesPacketReader& reader)
{
    g_logger.trace("%s", __func__);

    if (database.getPage().getState() != Page::State::INCOMPLETE)
    {
        g_logger.trace("%s - page not currently being parsed", __func__);
        return;
    }

    // copy parsed display definition as current
    // (may be just 'reset' if there was no DDS)
    auto& parsedDisplay = database.getParsedDisplay();
    database.getCurrentDisplay().set(parsedDisplay.getVersion(),
            parsedDisplay.getDisplayBounds(), parsedDisplay.getWindowBounds());

    // reset the parsed state to force parsing
    parsedDisplay.reset();

    database.getPage().finishParsing();
    database.commitPage();
}

} // namespace dvbsubdecoder
