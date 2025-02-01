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


#ifndef DVBSUBDECODER_PARSERODS_HPP_
#define DVBSUBDECODER_PARSERODS_HPP_

namespace dvbsubdecoder
{

class Region;
class Database;
class PesPacketReader;
class ObjectInstance;

/**
 * Parser - object data.
 */
class ParserODS
{
public:
    /**
     * Parses ODS.
     *
     * @param database
     *      Database to use.
     * @param reader
     *      Reader to segment data.
     */
    void parseObjectDataSegment(Database& database,
                                PesPacketReader& reader);

private:
    /**
     * Parses object pixels.
     *
     * @param nonModifyingColourFlag
     *      Flag indicating if non-modifying color mode is used.
     * @param region
     *      Region for which object is decoded.
     * @param object
     *      Object being decoded.
     * @param reader
     *      Reader to object data.
     */
    void parseObjectPixels(bool nonModifyingColourFlag,
                           Region& region,
                           const ObjectInstance& object,
                           PesPacketReader& reader);
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_PARSERODS_HPP_*/
