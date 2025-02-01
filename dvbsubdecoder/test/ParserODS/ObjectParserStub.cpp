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


#include "ObjectParserStub.hpp"

namespace
{

int g_callCount = 0;

}

void ObjectParserStub::resetCallCounter()
{
    g_callCount = 0;
}

bool ObjectParserStub::wasCalled()
{
    return g_callCount > 0;
}

namespace dvbsubdecoder
{

ObjectParser::ObjectParser(PesPacketReader& reader,
                           PixelWriter& writer) :
        m_reader(reader),
        m_writer(writer),
        m_currentMap(nullptr)
{
    // noop
}

void ObjectParser::parse()
{
    ++g_callCount;
}

} // namespace dvbsubdecoder
