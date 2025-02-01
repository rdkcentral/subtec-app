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


#include "DefaultCharsets.hpp"

namespace ttxdecoder
{

/**
 * National subsets characters data.
 */
const NationalCharactersArray NATIONAL_SUBSETS[NATIONAL_OPTIONS] =
{
  /* Czech/Slovak*/
  {{
    { 0x0023, Charset::G0_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x016F, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_RING },
    { 0x010D, Charset::G0_LATIN, 0x63, Property::VALUE_DIACRITIC_CARON },
    { 0x0165, Charset::G0_LATIN, 0x74, Property::VALUE_DIACRITIC_CARON },
    { 0x017E, Charset::G0_LATIN, 0x7A, Property::VALUE_DIACRITIC_CARON },
    { 0x00FD, Charset::G0_LATIN, 0x79, Property::VALUE_DIACRITIC_ACUTE },
    { 0x00ED, Charset::G0_LATIN, 0x69, Property::VALUE_DIACRITIC_GRAVE },
    { 0x0159, Charset::G0_LATIN, 0x72, Property::VALUE_DIACRITIC_CARON },
    { 0x00E9, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_ACUTE },
    { 0x00E1, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_ACUTE },
    { 0x011B, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_CARON },
    { 0x00FA, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_ACUTE },
    { 0x0161, Charset::G0_LATIN, 0x73, Property::VALUE_DIACRITIC_CARON }
  }},
  /* English */
  {{
    { 0x00A3, Charset::G2_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x0024, Charset::G2_LATIN, 0x24, Property::VALUE_DIACRITIC_NONE },
    { 0x0040, Charset::G0_LATIN, 0x40, Property::VALUE_DIACRITIC_NONE },
    { 0x2190, Charset::G2_LATIN, 0x2C, Property::VALUE_DIACRITIC_NONE },
    { 0x00BD, Charset::G2_LATIN, 0x3D, Property::VALUE_DIACRITIC_NONE },
    { 0x2192, Charset::G2_LATIN, 0x2E, Property::VALUE_DIACRITIC_NONE },
    { 0x2191, Charset::G2_LATIN, 0x2D, Property::VALUE_DIACRITIC_NONE },
    { 0x0023, Charset::G0_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x2014, Charset::G2_LATIN, 0x50, Property::VALUE_DIACRITIC_NONE },
    { 0x00BC, Charset::G2_LATIN, 0x3C, Property::VALUE_DIACRITIC_NONE },
    { 0x2016, Charset::G2_LATIN, 0x59, Property::VALUE_DIACRITIC_NONE },
    { 0x00BE, Charset::G2_LATIN, 0x3E, Property::VALUE_DIACRITIC_NONE },
    { 0x00F7, Charset::G2_LATIN, 0x38, Property::VALUE_DIACRITIC_NONE }
  }},
  /* Estonian */
  {{
    { 0x0023, Charset::G0_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x00F5, Charset::G0_LATIN, 0x6F, Property::VALUE_DIACRITIC_TILDE },
    { 0x0160, Charset::G0_LATIN, 0x53, Property::VALUE_DIACRITIC_CARON },
    { 0x00C4, Charset::G0_LATIN, 0x41, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00D6, Charset::G0_LATIN, 0x4F, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x017D, Charset::G0_LATIN, 0x5A, Property::VALUE_DIACRITIC_CARON },
    { 0x00DC, Charset::G0_LATIN, 0x55, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00D5, Charset::G0_LATIN, 0x4F, Property::VALUE_DIACRITIC_TILDE },
    { 0x0161, Charset::G0_LATIN, 0x73, Property::VALUE_DIACRITIC_CARON },
    { 0x00E4, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00F6, Charset::G0_LATIN, 0x6F, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x017E, Charset::G0_LATIN, 0x7A, Property::VALUE_DIACRITIC_CARON },
    { 0x00FC, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_UMLAUT }
  }},
  /* French */
  {{
    { 0x00E9, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_ACUTE },
    { 0x00EF, Charset::G0_LATIN, 0x69, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00E0, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_GRAVE },
    { 0x00EB, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00EA, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_CIRCUMFLEX },
    { 0x00F9, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_GRAVE },
    { 0x00EE, Charset::G0_LATIN, 0x69, Property::VALUE_DIACRITIC_CIRCUMFLEX },
    { 0x0023, Charset::G0_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x00E8, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_GRAVE },
    { 0x00E2, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_CIRCUMFLEX },
    { 0x00F4, Charset::G0_LATIN, 0x6F, Property::VALUE_DIACRITIC_CIRCUMFLEX },
    { 0x00FB, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_CIRCUMFLEX },
    { 0x00E7, Charset::G0_LATIN, 0x63, Property::VALUE_DIACRITIC_CEDILLA }
  }},
  /* German */
  {{
    { 0x0023, Charset::G0_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x0024, Charset::G2_LATIN, 0x24, Property::VALUE_DIACRITIC_NONE },
    { 0x00A7, Charset::G2_LATIN, 0x27, Property::VALUE_DIACRITIC_NONE },
    { 0x00C4, Charset::G0_LATIN, 0x41, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00D6, Charset::G0_LATIN, 0x4F, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00DC, Charset::G0_LATIN, 0x55, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x005E, Charset::G0_LATIN, 0x5E, Property::VALUE_DIACRITIC_NONE },
    { 0x005F, Charset::G0_LATIN, 0x5F, Property::VALUE_DIACRITIC_NONE },
    { 0x00B0, Charset::G2_LATIN, 0x30, Property::VALUE_DIACRITIC_NONE },
    { 0x00E4, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00F6, Charset::G0_LATIN, 0x6F, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00FC, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00DF, Charset::G2_LATIN, 0x7B, Property::VALUE_DIACRITIC_NONE },
  }},
  /* Italian */
  {{
    { 0x00A3, Charset::G2_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x0024, Charset::G2_LATIN, 0x24, Property::VALUE_DIACRITIC_NONE },
    { 0x00E9, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_ACUTE },
    { 0x00B0, Charset::G2_LATIN, 0x30, Property::VALUE_DIACRITIC_NONE },
    { 0x00E7, Charset::G0_LATIN, 0x63, Property::VALUE_DIACRITIC_CEDILLA },
    { 0x2192, Charset::G2_LATIN, 0x2E, Property::VALUE_DIACRITIC_NONE },
    { 0x2191, Charset::G2_LATIN, 0x2D, Property::VALUE_DIACRITIC_NONE },
    { 0x0023, Charset::G0_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x00F9, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_GRAVE },
    { 0x00E0, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_GRAVE },
    { 0x00F2, Charset::G0_LATIN, 0x6F, Property::VALUE_DIACRITIC_GRAVE },
    { 0x00E8, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_GRAVE },
    { 0x00EC, Charset::G0_LATIN, 0x69, Property::VALUE_DIACRITIC_GRAVE },
  }},
  /* Lettish/Lithuanian */
  {{
    { 0x0023, Charset::G0_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x0024, Charset::G2_LATIN, 0x24, Property::VALUE_DIACRITIC_NONE },
    { 0x0160, Charset::G0_LATIN, 0x53, Property::VALUE_DIACRITIC_CARON },
    { 0x0117, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_DOT_ABOVE },
    { 0x0229, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_CEDILLA },
    { 0x017D, Charset::G0_LATIN, 0x5A, Property::VALUE_DIACRITIC_CARON },
    { 0x010D, Charset::G0_LATIN, 0x63, Property::VALUE_DIACRITIC_CARON },
    { 0x016B, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_MACRON },
    { 0x0161, Charset::G0_LATIN, 0x73, Property::VALUE_DIACRITIC_CARON },
    { 0x0105, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_OGONEK },
    { 0x0173, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_OGONEK },
    { 0x017E, Charset::G0_LATIN, 0x7A, Property::VALUE_DIACRITIC_CARON },
    { 0x012F, Charset::G0_LATIN, 0x69, Property::VALUE_DIACRITIC_OGONEK },
  }},
  /* Polish */
  {{
    { 0x0023, Charset::G0_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x0144, Charset::G0_LATIN, 0x6E, Property::VALUE_DIACRITIC_ACUTE },
    { 0x0105, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_OGONEK },
    { 0x01B5, Charset::G2_LATIN, 0x5A, Property::VALUE_DIACRITIC_NONE },
    { 0x015A, Charset::G0_LATIN, 0x53, Property::VALUE_DIACRITIC_ACUTE },
    { 0x0141, Charset::G2_LATIN, 0x68, Property::VALUE_DIACRITIC_NONE },
    { 0x0107, Charset::G0_LATIN, 0x63, Property::VALUE_DIACRITIC_ACUTE },
    { 0x00F3, Charset::G0_LATIN, 0x6F, Property::VALUE_DIACRITIC_ACUTE },
    { 0x0119, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_OGONEK },
    { 0x017C, Charset::G0_LATIN, 0x7A, Property::VALUE_DIACRITIC_DOT_ABOVE },
    { 0x015B, Charset::G0_LATIN, 0x73, Property::VALUE_DIACRITIC_ACUTE },
    { 0x0142, Charset::G2_LATIN, 0x78, Property::VALUE_DIACRITIC_NONE },
    { 0x017A, Charset::G0_LATIN, 0x7A, Property::VALUE_DIACRITIC_ACUTE },
  }},
  /* Spanish */
  {{
    { 0x00E7, Charset::G0_LATIN, 0x63, Property::VALUE_DIACRITIC_CEDILLA },
    { 0x0024, Charset::G2_LATIN, 0x24, Property::VALUE_DIACRITIC_NONE },
    { 0x00A1, Charset::G2_LATIN, 0x21, Property::VALUE_DIACRITIC_NONE },
    { 0x00E1, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_ACUTE },
    { 0x00E9, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_ACUTE },
    { 0x00ED, Charset::G0_LATIN, 0x69, Property::VALUE_DIACRITIC_ACUTE },
    { 0x00F3, Charset::G0_LATIN, 0x6F, Property::VALUE_DIACRITIC_ACUTE },
    { 0x00FA, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_ACUTE },
    { 0x00BF, Charset::G2_LATIN, 0x3F, Property::VALUE_DIACRITIC_NONE },
    { 0x00FC, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00F1, Charset::G0_LATIN, 0x6E, Property::VALUE_DIACRITIC_TILDE },
    { 0x00E8, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_GRAVE },
    { 0x00E0, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_GRAVE },
  }},
  /* Rumanian */
  {{
    { 0x0023, Charset::G0_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x00A4, Charset::G0_LATIN, 0x24, Property::VALUE_DIACRITIC_NONE },
    { 0x0162, Charset::G0_LATIN, 0x54, Property::VALUE_DIACRITIC_CEDILLA },
    { 0x00C2, Charset::G0_LATIN, 0x41, Property::VALUE_DIACRITIC_CIRCUMFLEX },
    { 0x015E, Charset::G0_LATIN, 0x53, Property::VALUE_DIACRITIC_CEDILLA },
    { 0x01CD, Charset::G0_LATIN, 0x41, Property::VALUE_DIACRITIC_CARON },
    { 0x00CD, Charset::G0_LATIN, 0x49, Property::VALUE_DIACRITIC_ACUTE },
    { 0x0131, Charset::G2_LATIN, 0x75, Property::VALUE_DIACRITIC_NONE },
    { 0x0163, Charset::G0_LATIN, 0x74, Property::VALUE_DIACRITIC_CEDILLA },
    { 0x00E2, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_CIRCUMFLEX },
    { 0x015F, Charset::G0_LATIN, 0x73, Property::VALUE_DIACRITIC_CEDILLA },
    { 0x01CE, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_CARON },
    { 0x00EE, Charset::G0_LATIN, 0x69, Property::VALUE_DIACRITIC_CIRCUMFLEX },
  }},
  /* Serbian/Croatian/Slovenian */
  {{
    { 0x0023, Charset::G0_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x00CB, Charset::G0_LATIN, 0x45, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x010C, Charset::G0_LATIN, 0x43, Property::VALUE_DIACRITIC_CARON },
    { 0x0106, Charset::G0_LATIN, 0x43, Property::VALUE_DIACRITIC_ACUTE },
    { 0x017D, Charset::G0_LATIN, 0x5A, Property::VALUE_DIACRITIC_CARON },
    { 0x00D0, Charset::G2_LATIN, 0x62, Property::VALUE_DIACRITIC_NONE },
    { 0x0160, Charset::G0_LATIN, 0x53, Property::VALUE_DIACRITIC_CARON },
    { 0x00EB, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x010D, Charset::G0_LATIN, 0x63, Property::VALUE_DIACRITIC_CARON },
    { 0x0107, Charset::G0_LATIN, 0x63, Property::VALUE_DIACRITIC_ACUTE },
    { 0x017E, Charset::G0_LATIN, 0x7A, Property::VALUE_DIACRITIC_CARON },
    { 0x00F0, Charset::G2_LATIN, 0x73, Property::VALUE_DIACRITIC_NONE },
    { 0x0161, Charset::G0_LATIN, 0x73, Property::VALUE_DIACRITIC_CARON },
  }},
  /* Swedish/Finnish */
  {{
    { 0x0023, Charset::G0_LATIN, 0x23, Property::VALUE_DIACRITIC_NONE },
    { 0x00A4, Charset::G0_LATIN, 0x24, Property::VALUE_DIACRITIC_NONE },
    { 0x00C9, Charset::G0_LATIN, 0x45, Property::VALUE_DIACRITIC_ACUTE },
    { 0x00C4, Charset::G0_LATIN, 0x41, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00D6, Charset::G0_LATIN, 0x4F, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00C5, Charset::G0_LATIN, 0x41, Property::VALUE_DIACRITIC_RING },
    { 0x00DC, Charset::G0_LATIN, 0x55, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x005F, Charset::G0_LATIN, 0x5F, Property::VALUE_DIACRITIC_NONE },
    { 0x00E9, Charset::G0_LATIN, 0x65, Property::VALUE_DIACRITIC_ACUTE },
    { 0x00E4, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00F6, Charset::G0_LATIN, 0x6F, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00E5, Charset::G0_LATIN, 0x61, Property::VALUE_DIACRITIC_RING },
    { 0x00FC, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_UMLAUT },
  }},
  /* Turkish */
  {{
    { 0xE800, Charset::G2_LATIN, 0x5B, Property::VALUE_DIACRITIC_NONE },
    { 0x011F, Charset::G0_LATIN, 0x67, Property::VALUE_DIACRITIC_BREVE },
    { 0x0130, Charset::G0_LATIN, 0x49, Property::VALUE_DIACRITIC_DOT_ABOVE },
    { 0x015E, Charset::G0_LATIN, 0x53, Property::VALUE_DIACRITIC_CEDILLA },
    { 0x00D6, Charset::G0_LATIN, 0x4F, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00C7, Charset::G0_LATIN, 0x43, Property::VALUE_DIACRITIC_CEDILLA },
    { 0x00DC, Charset::G0_LATIN, 0x55, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x011E, Charset::G0_LATIN, 0x47, Property::VALUE_DIACRITIC_BREVE },
    { 0x0131, Charset::G2_LATIN, 0x75, Property::VALUE_DIACRITIC_NONE },
    { 0x015F, Charset::G0_LATIN, 0x73, Property::VALUE_DIACRITIC_CEDILLA },
    { 0x00F6, Charset::G0_LATIN, 0x6F, Property::VALUE_DIACRITIC_UMLAUT },
    { 0x00E7, Charset::G0_LATIN, 0x63, Property::VALUE_DIACRITIC_CEDILLA },
    { 0x00FC, Charset::G0_LATIN, 0x75, Property::VALUE_DIACRITIC_UMLAUT },
  }}
};

} // namespace ttxdecoder
