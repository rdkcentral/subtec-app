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


#include "CharsetHandler.hpp"

#include <set>

#include <ttxdecoder/Property.hpp>

namespace subttxrend
{
namespace ttxt
{

namespace
{

void addMappedChars(std::set<std::uint16_t>& mappedCharsSet,
                    const ttxdecoder::Engine& engine,
                    ttxdecoder::Charset charset)
{
    const auto& mappedCharsArray = engine.getCharsetMapping(charset);
    mappedCharsSet.insert(mappedCharsArray.begin(), mappedCharsArray.end());
}

void addMappedDiacritic(std::set<std::uint16_t>& mappedCharsSet,
                        const std::uint16_t character,
                        const std::uint16_t diacriticProperty)
{
    auto mappedCharacter = CharsetHandler::getDiacriticCharacterCode(character,
            diacriticProperty);
    mappedCharsSet.insert(mappedCharacter);
}

void addMappedDiacritic(std::set<std::uint16_t>& mappedCharsSet,
                        const std::vector<std::uint16_t>& baseChars,
                        const std::uint16_t diacriticProperty)
{
    for (const auto& ch : baseChars)
    {
        addMappedDiacritic(mappedCharsSet, ch, diacriticProperty);
    }
}

} // namespace anonymous

CharsetHandler::CharsetHandler()
{
    // noop
}

void CharsetHandler::init(const ttxdecoder::Engine& engine)
{
    std::set<std::uint16_t> mappedChars;

    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G0_LATIN);
    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G0_ARABIC);
    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G0_CYRILLIC1);
    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G0_CYRILLIC2);
    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G0_CYRILLIC3);
    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G0_GREEK);
    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G0_HEBREW);
    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G0_FARSI);
    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G2_LATIN);
    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G2_ARABIC);
    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G2_CYRILLIC);
    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G2_GREEK);
    addMappedChars(mappedChars, engine, ttxdecoder::Charset::G2_FARSI);

    std::vector<std::uint16_t> baseChars(mappedChars.begin(),
            mappedChars.end());

    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_TILDE);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_MACRON);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_BREVE);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_DOT_BELOW);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_RING);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_UNDERLINE);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_DOUBLE_ACUTE);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_OGONEK);
    addMappedDiacritic(mappedChars, baseChars,
            ttxdecoder::Property::VALUE_DIACRITIC_CARON);

    // at this point we should have all characters

    m_mapping.clear();
    std::uint16_t index = 0;
    for (const auto& character : mappedChars)
    {
        m_mapping.addMapping(character, index++);
    }
}

void CharsetHandler::shutdown()
{
    // noop
}

const gfx::FontStripMap& CharsetHandler::getMapping() const
{
    return m_mapping;
}

std::uint16_t CharsetHandler::getDiacriticCharacterCode(std::uint16_t character,
                                                        std::uint16_t diacriticProperty)
{
    switch (character)
    {
    case 0x0041: /* 'A' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x00C0;
            break; /* Capital A grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x00C1;
            break; /* Capital A acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x00C2;
            break; /* Capital A circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_TILDE:
            character = 0x00C3;
            break; /* Capital A tilde */
        case ttxdecoder::Property::VALUE_DIACRITIC_MACRON:
            character = 0x0100;
            break; /* Capital A macron */
        case ttxdecoder::Property::VALUE_DIACRITIC_BREVE:
            character = 0x0102;
            break; /* Capital A breve */
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x00C4;
            break; /* Capital A diaresis (umlaut) */
        case ttxdecoder::Property::VALUE_DIACRITIC_RING:
            character = 0x00C5;
            break; /* Capital A ring */
        case ttxdecoder::Property::VALUE_DIACRITIC_OGONEK:
            character = 0x0104;
            break; /* Capital A ogonek */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x01CD;
            break; /* Capital A caron */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x0226;
            break; /* Capital A dot above */
        default:
            break;
        }
        break;
    case 0x0042: /* 'B' */
        break;
    case 0x0043: /* 'C' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x0106;
            break; /* Capital C acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x0108;
            break; /* Capital C circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x010A;
            break; /* Capital C dot above */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x00C7;
            break; /* Capital C cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x010C;
            break; /* Capital C caron */
        default:
            break;
        }
        break;
    case 0x0044: /* 'D' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x010E;
            break; /* Capital D caron */
        default:
            break;
        }
        break;
    case 0x0045: /* 'E' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x00C8;
            break; /* Capital E grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x00C9;
            break; /* Capital E acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x00CA;
            break; /* Capital E circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_MACRON:
            character = 0x0112;
            break; /* Capital E macron */
        case ttxdecoder::Property::VALUE_DIACRITIC_BREVE:
            character = 0x0114;
            break; /* Capital E breve */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x0116;
            break; /* Capital E dot above */
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x00CB;
            break; /* Capital E diaresis (umlaut) */
        case ttxdecoder::Property::VALUE_DIACRITIC_OGONEK:
            character = 0x0118;
            break; /* Capital E ogonek */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x011A;
            break; /* Capital E caron */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x0228;
            break; /* Capital E cedilla */
        default:
            break;
        }
        break;
    case 0x0046: /* 'F' */
        break;
    case 0x0047: /* 'G' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x01F4;
            break; /* Capital G acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x011C;
            break; /* Capital G circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_BREVE:
            character = 0x011E;
            break; /* Capital G breve */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x0120;
            break; /* Capital G dot above */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x0122;
            break; /* Capital G cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x01E6;
            break; /* Capital G caron */
        default:
            break;
        }
        break;
    case 0x0048: /* 'H' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x0124;
            break; /* Capital H circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x021E;
            break; /* Capital H caron */
        default:
            break;
        }
        break;
    case 0x0049: /* 'I' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x00CC;
            break; /* Capital I grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x00CD;
            break; /* Capital I acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x00CE;
            break; /* Capital I circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_MACRON:
            character = 0x012A;
            break; /* Capital I macron */
        case ttxdecoder::Property::VALUE_DIACRITIC_BREVE:
            character = 0x012C;
            break; /* Capital I breve */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x0130;
            break; /* Capital I dot above */
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x00CF;
            break; /* Capital I diaresis (umlaut) */
        case ttxdecoder::Property::VALUE_DIACRITIC_OGONEK:
            character = 0x012E;
            break; /* Capital I ogonek */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x01CF;
            break; /* Capital I caron */
        case ttxdecoder::Property::VALUE_DIACRITIC_TILDE:
            character = 0x0128;
            break; /* Capital I tilde */
        default:
            break;
        }
        break;
    case 0x004a: /* 'J' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x0134;
            break; /* Capital J circumflex */
        default:
            break;
        }
        break;
    case 0x004b: /* 'K' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x0136;
            break; /* Capital K cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x01E8;
            break; /* Capital K caron */
        default:
            break;
        }
        break;
    case 0x004c: /* 'L' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x0139;
            break; /* Capital L acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x013B;
            break; /* Capital L cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x013D;
            break; /* Capital L caron */
        default:
            break;
        }
        break;
    case 0x004d: /* 'M' */
        break;
    case 0x004e: /* 'N' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x01F8;
            break; /* Capital N grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x0143;
            break; /* Capital N acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_TILDE:
            character = 0x00D1;
            break; /* Capital N tilde */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x0145;
            break; /* Capital N cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x0147;
            break; /* Capital N caron */
        default:
            break;
        }
        break;
    case 0x004f: /* 'O' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x00D2;
            break; /* Capital O grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x00D3;
            break; /* Capital O acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x00D4;
            break; /* Capital O circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_TILDE:
            character = 0x00D5;
            break; /* Capital O tilde */
        case ttxdecoder::Property::VALUE_DIACRITIC_MACRON:
            character = 0x014C;
            break; /* Capital O macron */
        case ttxdecoder::Property::VALUE_DIACRITIC_BREVE:
            character = 0x014E;
            break; /* Capital O breve */
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x00D6;
            break; /* Capital O diaresis (umlaut) */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOUBLE_ACUTE:
            character = 0x0150;
            break; /* Capital O double acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x01D1;
            break; /* Capital O caron */
        case ttxdecoder::Property::VALUE_DIACRITIC_OGONEK:
            character = 0x01EA;
            break; /* Capital O ogonek */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x022E;
            break; /* Capital O dot above */
        default:
            break;
        }
        break;
    case 0x0050: /* 'P' */
        break;
    case 0x0051: /* 'Q' */
        break;
    case 0x0052: /* 'R' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x0154;
            break; /* Capital R acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x0156;
            break; /* Capital R cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x0158;
            break; /* Capital R caron */
        default:
            break;
        }
        break;
    case 0x0053: /* 'S' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x015A;
            break; /* Capital S acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x015C;
            break; /* Capital S circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x015E;
            break; /* Capital S cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x0160;
            break; /* Capital S caron */
        default:
            break;
        }
        break;
    case 0x0054: /* 'T' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x0162;
            break; /* Capital T cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x0164;
            break; /* Capital T caron */
        default:
            break;
        }
        break;
    case 0x0055: /* 'U' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x00D9;
            break; /* Capital U grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x00DA;
            break; /* Capital U acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x00DB;
            break; /* Capital U circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_TILDE:
            character = 0x0168;
            break; /* Capital U tilde */
        case ttxdecoder::Property::VALUE_DIACRITIC_MACRON:
            character = 0x016A;
            break; /* Capital U macron */
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x00DC;
            break; /* Capital U diaresis (umlaut) */
        case ttxdecoder::Property::VALUE_DIACRITIC_RING:
            character = 0x016E;
            break; /* Capital U ring */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOUBLE_ACUTE:
            character = 0x0170;
            break; /* Capital U double acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_OGONEK:
            character = 0x0172;
            break; /* Capital U ogonek */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x01D3;
            break; /* Capital U caron */
        case ttxdecoder::Property::VALUE_DIACRITIC_BREVE:
            character = 0x016C;
            break; /* Capital U breve */
        default:
            break;
        }
        break;
    case 0x0056: /* 'V' */
        break;
    case 0x0057: /* 'W' */
        switch (diacriticProperty)
        {
         case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x1E82;
            break; /* Capital W acute*/
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x0174;
            break; /* Capital W circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
           character = 0x1E80;
           break; /* Capital W grave*/
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x1E84;
            break; /* Capital W diaresis (umlaut) */
        default:
            break;
        }
        break;
    case 0x0058: /* 'X' */
        break;
    case 0x0059: /* 'Y' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x1EF2;
            break; /* Capital Y grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x00DD;
            break; /* Capital Y acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x0176;
            break; /* Capital Y circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_TILDE:
            character = 0x1EF8;
            break; /* Capital Y tilde */
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x0178;
            break; /* Capital Y diaresis (umlaut) */
        case ttxdecoder::Property::VALUE_DIACRITIC_MACRON:
            character = 0x0232;
            break; /* Capital Y macron */
        default:
            break;
        }
        break;
    case 0x005a: /* 'Z' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x0179;
            break; /* Capital Z acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x017B;
            break; /* Capital Z dot above */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x017D;
            break; /* Capital Z caron */
        default:
            break;
        }
        break;
    case 0x0061: /* 'a' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x00E0;
            break; /* Small a grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x00E1;
            break; /* Small a acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x00E2;
            break; /* Small a circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_TILDE:
            character = 0x00E3;
            break; /* Small a tilde */
        case ttxdecoder::Property::VALUE_DIACRITIC_MACRON:
            character = 0x0101;
            break; /* Small a macron */
        case ttxdecoder::Property::VALUE_DIACRITIC_BREVE:
            character = 0x0103;
            break; /* Small a breve */
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x00E4;
            break; /* Small a diaresis (umlaut) */
        case ttxdecoder::Property::VALUE_DIACRITIC_RING:
            character = 0x00E5;
            break; /* Small a ring */
        case ttxdecoder::Property::VALUE_DIACRITIC_OGONEK:
            character = 0x0105;
            break; /* Small a ogonek */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x01CE;
            break; /* Small a caron */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x0227;
            break; /* Small a dot above */
        default:
            break;
        }
        break;
    case 0x0062: /* 'b' */
        switch (diacriticProperty)
        {
        default:
            break;
        }
        break;
    case 0x0063: /* 'c' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x0107;
            break; /* Small c acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x0109;
            break; /* Small c circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x010B;
            break; /* Small c dot above */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x00E7;
            break; /* Small c cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x010D;
            break; /* Small c caron */
        default:
            break;
        }
        break;
    case 0x0064: /* 'd' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x010F;
            break; /* Small d caron */
        default:
            break;
        }
        break;
    case 0x0065: /* 'e' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x00E8;
            break; /* Small e grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x00E9;
            break; /* Small e acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x00EA;
            break; /* Small e circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_MACRON:
            character = 0x0113;
            break; /* Small e macron */
        case ttxdecoder::Property::VALUE_DIACRITIC_BREVE:
            character = 0x0115;
            break; /* Small e breve */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x0117;
            break; /* Small e dot above */
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x00EB;
            break; /* Small e diaresis (umlaut) */
        case ttxdecoder::Property::VALUE_DIACRITIC_OGONEK:
            character = 0x0119;
            break; /* Small e ogonek */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x011B;
            break; /* Small e caron */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x0229;
            break; /* Small e cedilla */
        default:
            break;
        }
        break;
    case 0x0066: /* 'f' */
        switch (diacriticProperty)
        {
        default:
            break;
        }
        break;
    case 0x0067: /* 'g' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x01F5;
            break; /* Small g acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x011D;
            break; /* Small g circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_BREVE:
            character = 0x011F;
            break; /* Small g breve */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x0121;
            break; /* Small g dot above */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x01E7;
            break; /* Small g caron */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x0123;
            break; /* Small g cedilla */
        default:
            break;
        }
        break;
    case 0x0068: /* 'h' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x0125;
            break; /* Small h circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x021F;
            break; /* Small h caron */
        default:
            break;
        }
        break;
    case 0x0069: /* 'i' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x00EC;
            break; /* Small i grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x00ED;
            break; /* Small i acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x00EE;
            break; /* Small i circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_TILDE:
            character = 0x0129;
            break; /* Small i tilde */
        case ttxdecoder::Property::VALUE_DIACRITIC_MACRON:
            character = 0x012B;
            break; /* Small i macron */
        case ttxdecoder::Property::VALUE_DIACRITIC_BREVE:
            character = 0x012D;
            break; /* Small i breve */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x0069;
            break; /* Small i */
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x00EF;
            break; /* Small i diaresis (umlaut) */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x01D0;
            break; /* Small i caron */
        case ttxdecoder::Property::VALUE_DIACRITIC_OGONEK:
            character = 0x012F;
            break; /* Small i ogonek */
        default:
            break;
        }
        break;
    case 0x006a: /* 'j' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x0135;
            break; /* Small j circumflex */
        default:
            break;
        }
        break;
    case 0x006b: /* 'k' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x0137;
            break; /* Small k cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x01E9;
            break; /* Small k caron */
        default:
            break;
        }
        break;
    case 0x006c: /* 'l' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x013A;
            break; /* Small l acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x013C;
            break; /* Small l cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x013E;
            break; /* Small l caron */
        default:
            break;
        }
        break;
    case 0x006d: /* 'm' */
        switch (diacriticProperty)
        {
        default:
            break;
        }
        break;
    case 0x006e: /* 'n' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x01F9;
            break; /* Small n grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x0144;
            break; /* Small n acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_TILDE:
            character = 0x00F1;
            break; /* Small n tilde */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x0146;
            break; /* Small n cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x0148;
            break; /* Small n caron */
        default:
            break;
        }
        break;
    case 0x006f: /* 'o' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x00F2;
            break; /* Small o grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x00F3;
            break; /* Small o acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x00F4;
            break; /* Small o circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_TILDE:
            character = 0x00F5;
            break; /* Small o tilde */
        case ttxdecoder::Property::VALUE_DIACRITIC_MACRON:
            character = 0x014D;
            break; /* Small o macron */
        case ttxdecoder::Property::VALUE_DIACRITIC_BREVE:
            character = 0x014F;
            break; /* Small o breve */
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x00F6;
            break; /* Small o diaresis (umlaut) */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOUBLE_ACUTE:
            character = 0x0151;
            break; /* Small o double acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x01D2;
            break; /* Small o caron */
        case ttxdecoder::Property::VALUE_DIACRITIC_OGONEK:
            character = 0x01EB;
            break; /* Small o ogonek */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x022F;
            break; /* Small o dot above */
        default:
            break;
        }
        break;
    case 0x0070: /* 'p' */
        switch (diacriticProperty)
        {
        default:
            break;
        }
        break;
    case 0x0071: /* 'q' */
        switch (diacriticProperty)
        {
        default:
            break;
        }
        break;
    case 0x0072: /* 'r' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x0155;
            break; /* Small r acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x0157;
            break; /* Small r cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x0159;
            break; /* Small r caron */
        default:
            break;
        }
        break;
    case 0x0073: /* 's' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x015B;
            break; /* Small s acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x015D;
            break; /* Small s circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x015F;
            break; /* Small s cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x0161;
            break; /* Small s caron */
        default:
            break;
        }
        break;
    case 0x0074: /* 't' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA:
            character = 0x0163;
            break; /* Small t cedilla */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x0165;
            break; /* Small t caron */
        default:
            break;
        }
        break;
    case 0x0075: /* 'u' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
            character = 0x00F9;
            break; /* Small u grave */
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x00FA;
            break; /* Small u acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x00FB;
            break; /* Small u circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_TILDE:
            character = 0x0169;
            break; /* Small u tilde */
        case ttxdecoder::Property::VALUE_DIACRITIC_MACRON:
            character = 0x016B;
            break; /* Small u macron */
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x00FC;
            break; /* Small u diaresis (umlaut) */
        case ttxdecoder::Property::VALUE_DIACRITIC_RING:
            character = 0x016F;
            break; /* Small u ring */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOUBLE_ACUTE:
            character = 0x0171;
            break; /* Small u double acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_OGONEK:
            character = 0x0173;
            break; /* Small u ogonek */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x01D4;
            break; /* Small u caron */
        case ttxdecoder::Property::VALUE_DIACRITIC_BREVE:
            character = 0x016D;
            break; /* Small u breve */
        default:
            break;
        }
        break;
    case 0x0076: /* 'v' */
        switch (diacriticProperty)
        {
        default:
            break;
        }
        break;
    case 0x0077: /* 'w' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
           character = 0x1E83;
           break; /* Small w acute*/
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x0175;
            break; /* Small w circumflex */
       case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
          character = 0x1E81;
          break; /* Small w grave*/
       case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
           character = 0x1E85;
           break; /* Small w diaresis (umlaut) */
        default:
            break;
        }
        break;
    case 0x0078: /* 'x' */
        switch (diacriticProperty)
        {
        default:
            break;
        }
        break;
    case 0x0079: /* 'y' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x00FD;
            break; /* Small y acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX:
            character = 0x0177;
            break; /* Small y circumflex */
        case ttxdecoder::Property::VALUE_DIACRITIC_GRAVE:
           character = 0x1EF3;
           break; /* Small y grave*/
        case ttxdecoder::Property::VALUE_DIACRITIC_TILDE:
            character = 0x1EF9;
            break; /* Small y tilde */
        case ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT:
            character = 0x00FF;
            break; /* Small y diaresis (umlaut) */
        case ttxdecoder::Property::VALUE_DIACRITIC_MACRON:
            character = 0x0233;
            break; /* Small y macron */
        default:
            break;
        }
        break;
    case 0x007a: /* 'z' */
        switch (diacriticProperty)
        {
        case ttxdecoder::Property::VALUE_DIACRITIC_ACUTE:
            character = 0x017A;
            break; /* Small z acute */
        case ttxdecoder::Property::VALUE_DIACRITIC_CARON:
            character = 0x017E;
            break; /* Small z caron */
        case ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE:
            character = 0x017C;
            break; /* Small z dot above */
        default:
            break;
        }
        break;
    }

    return character;
}

} // namespace ttxt
} // namespace subttxrend
