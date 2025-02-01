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


#ifndef TTXDECODER_PARSERROWCONTEXT_HPP_
#define TTXDECODER_PARSERROWCONTEXT_HPP_

#include <tuple>

#include "Property.hpp"
#include "CharacterMarker.hpp"
#include "CharsetMapping.hpp"

#include "Types.hpp"

namespace ttxdecoder
{

/**
 * Row parsing context.
 *
 * Contains row parsing state and character processing logic.
 */
class ParserRowContext
{
public:
    /**
     * Charsets.
     */
    enum class Charset
    {
        /** G0 */
        G0,
        /** G1 */
        G1,
    };

    /**
     * Charset mode.
     *
     * Used by G0 charset.
     */
    enum class CharsetMode
    {
        /** Standard charset. */
        STANDARD,
        /** Alternative charset. */
        ALTERNATIVE
    };

    /**
     * Mosaic mode.
     *
     * Used by G1 charset.
     */
    enum class MosaicMode
    {
        /** Contiguous mosaic. */
        CONTIGUOUS,
        /** Separate mosaic. */
        SEPARATED
    };

    /**
     * Character size mode.
     */
    enum class SizeMode
    {
        /** Normal. */
        NORMAL,
        /** Double width. */
        DOUBLE_WIDTH,
        /** Double height. */
        DOUBLE_HEIGHT,
        /** Double size (width & height). */
        DOUBLE_SIZE
    };

    /** Space character value. */
    static const uint8_t SPACE = 0x20;

    /** Default background color index. */
    static const uint8_t DEFAULT_BG_COLOR_INDEX = 0;

    /** Default foreground color index. */
    static const uint8_t DEFAULT_FG_COLOR_INDEX = 7;

    /**
     * Constructor.
     *
     * Resets the context to row initial state.
     *
     * @param primaryG0
     *      Primary G0/G2 charset.
     * @param secondaryG0
     *      Secondary G0 charset.
     * @param charsetConfig
     *      Charset configuration.
     * @param boxedMode
     *      Boxed mode (true for news/subtitles).
     */
    ParserRowContext(const NationalCharset primaryG0,
                     const NationalCharset secondaryG0,
                     const CharsetConfig& charsetConfig,
                     bool boxedMode) :
            m_charsetMapping(charsetConfig, primaryG0, secondaryG0),
            m_boxedMode(boxedMode),
            m_inBox(false),
            m_holdMosaics(false),
            m_holdMosaicCharacter(0),

            m_charset(Charset::G0),
            m_charsetMode(CharsetMode::STANDARD),
            m_mosaicMode(MosaicMode::CONTIGUOUS),
            m_sizeMode(SizeMode::NORMAL),

            m_character(SPACE),
            m_bgColorIndex(DEFAULT_BG_COLOR_INDEX),
            m_fgColorIndex(DEFAULT_FG_COLOR_INDEX),
            m_properties(0)
    {
        resetHoldMosaicCharacter();
    }

    /**
     * Switches G0 charset mode.
     */
    void switchG0Charset()
    {
        m_charsetMapping.switchG0Charset();
    }

    /**
     * Parsed value getter - character.
     *
     * @return
     *      Parsed value.
     */
    uint16_t getCharacter() const
    {
        return m_character;
    }

    /**
     * Parsed value getter - background color index.
     *
     * @return
     *      Parsed value.
     */
    uint8_t getBgColorIndex() const
    {
        return m_bgColorIndex;
    }

    /**
     * Parsed value getter - foreground color index.
     *
     * @return
     *      Parsed value.
     */
    uint8_t getFgColorIndex() const
    {
        return m_fgColorIndex;
    }

    /**
     * Parsed value getter - properties.
     *
     * @return
     *      Parsed value.
     */
    uint16_t getProperties() const
    {
        return m_properties;
    }

    /**
     * Sets foreground color and current charset.
     *
     * @param fgColorIndex
     *      Selected color index.
     * @param charset
     *      Selected charset.
     */
    void setFgColorAndCharset(uint8_t fgColorIndex,
                              Charset charset)
    {
        m_fgColorIndex = fgColorIndex;
        m_charset = charset;

        // reset on color code attribute
        clearProperty(Property::VALUE_CONCEALED);

        // not reset by a change in hold mosaics mode
        if (!m_holdMosaics)
        {
            resetHoldMosaicCharacter();
        }
    }

    /**
     * Sets character size mode.
     *
     * @param mode
     *      New character size mode.
     */
    void setSizeMode(SizeMode mode)
    {
        // hold mosaics not reset by reinforcement of the existing size settings
        // (and also nothing to do)
        if (m_sizeMode == mode)
        {
            return;
        }

        // not reset by a change in hold mosaics mode
        if (!m_holdMosaics)
        {
            resetHoldMosaicCharacter();
        }

        m_sizeMode = mode;

        clearProperty(Property::VALUE_DOUBLE_HEIGHT);
        clearProperty(Property::VALUE_DOUBLE_WIDTH);

        switch (m_sizeMode)
        {
        case SizeMode::NORMAL:
            // noop
            break;
        case SizeMode::DOUBLE_HEIGHT:
            setProperty(Property::VALUE_DOUBLE_HEIGHT);
            break;
        case SizeMode::DOUBLE_WIDTH:
            setProperty(Property::VALUE_DOUBLE_WIDTH);
            break;
        case SizeMode::DOUBLE_SIZE:
            setProperty(Property::VALUE_DOUBLE_HEIGHT);
            setProperty(Property::VALUE_DOUBLE_WIDTH);
            break;
        }
    }

    /**
     * Sets flash property.
     */
    void setFlash()
    {
        setProperty(Property::VALUE_FLASH);
    }

    /**
     * Unsets flash property.
     */
    void unsetFlash()
    {
        clearProperty(Property::VALUE_FLASH);
    }

    /**
     * Sets concealed property.
     */
    void setConcealed()
    {
        setProperty(Property::VALUE_CONCEALED);
    }

    /**
     * Starts box.
     *
     * Used to start showing characters in boxed mode.
     */
    void startBox()
    {
        m_inBox = true;
    }

    /**
     * Ends box.
     *
     * Used to stop showing characters in boxed mode.
     */
    void endBox()
    {
        m_inBox = false;
    }

    /**
     * Enables "hold mosaics" mode.
     */
    void holdMosaics()
    {
        m_holdMosaics = true;
    }

    /**
     * Disables "hold mosaics" mode.
     */
    void releaseMosaics()
    {
        m_holdMosaics = false;
        setMosaicModeCharset();
    }

    /**
     * Stores character.
     *
     * Performs logic related to actual mode settings and
     * stores the new parsed character value.
     *
     * @param character
     *      Character value.
     *      Use zero for spacing.
     */
    void storeCharacter(uint16_t character)
    {
        // store mosaics
        if (m_charset == Charset::G1)
        {
            // store most recent mosaics character with bit 6 = '1'
            if ((character & 0x20) != 0)
            {
                setHoldMosaicCharacter(character);
            }
        }

        // make sure control code are not displayed
        if (character < SPACE)
        {
            // hold mosaics
            if ((m_charset == Charset::G1) && m_holdMosaics)
            {
                character = m_holdMosaicCharacter;
            }
            else
            {
                character = SPACE;
            }
        }

        // disable rendering if boxed mode is on and character is not in a box
        if (m_boxedMode)
        {
            if (!m_inBox)
            {
                // no matter if mosaic, shall be invisible
                character = SPACE;
                setProperty(Property::VALUE_HIDDEN);
            }
            else
            {
                clearProperty(Property::VALUE_HIDDEN);
            }
        }

        uint16_t mappedCharacter = SPACE;
        uint16_t diacriticProperty = Property::VALUE_DIACRITIC_NONE;

        if (m_charset == Charset::G0)
        {
            std::tie(mappedCharacter, diacriticProperty) =
                    m_charsetMapping.mapG0Character(character);
        }
        else
        {
            std::tie(mappedCharacter, diacriticProperty) =
                    m_charsetMapping.mapG1Character(character);

            if (mappedCharacter == 0x0000)
            {
                std::tie(mappedCharacter, diacriticProperty) =
                        m_charsetMapping.mapG0Character(character);
            }
        }

//#define VERBOSE_DEBUG
#ifdef VERBOSE_DEBUG
        subttxrend::common::Logger g_logger("TtxDecoder", "ParserRowContext");
        g_logger.info("%s - char=%u -> char=%u,prop=%u", __func__, character, mappedCharacter, diacriticProperty);
#endif

        m_character = mappedCharacter;

        // disable rendering if boxed mode is on and character is not in a box
        if (m_boxedMode)
        {
            if (!m_inBox)
            {
                // no matter if mosaic, shall be invisible
                m_character = 0;
            }
        }

        clearProperty(Property::MASK_DIACRITIC);
        setProperty(diacriticProperty);
    }

    /**
     * Sets mosaic mode.
     *
     * @param mode
     *      Mosaic mode to set.
     */
    void setMosaicMode(MosaicMode mode)
    {
        m_mosaicMode = mode;

        // not reset by a change in hold mosaics mode
        if (!m_holdMosaics)
        {
            resetHoldMosaicCharacter();
            setMosaicModeCharset();
        }
    }

    /**
     * Sets background color to black.
     */
    void setBlackBackground()
    {
        m_bgColorIndex = 0;
    }

    /**
     * Sets background color to foreground color.
     */
    void setNewBackground()
    {
        m_bgColorIndex = m_fgColorIndex;
    }

private:

    /**
     * Setups CharsetMapping to use chosen mosaic charset.
     */
    void setMosaicModeCharset()
    {
        ttxdecoder::Charset g1Charset = (m_mosaicMode == MosaicMode::CONTIGUOUS) ?
                ttxdecoder::Charset::G1_BLOCK_MOSAIC : ttxdecoder::Charset::G1_BLOCK_MOSAIC_SEPARATED;

        m_charsetMapping.setCurrentG1(g1Charset);
    }

    /**
     * Resets "hold mosaic" character to default.
     */
    void resetHoldMosaicCharacter()
    {
        setHoldMosaicCharacter(SPACE);
    }

    /**
     * Sets "hold mosaic" character.
     *
     * @param character
     *      Character to set.
     */
    void setHoldMosaicCharacter(uint8_t character)
    {
        m_holdMosaicCharacter = character;
    }

    /**
     * Sets property.
     *
     * @param propertyValue
     *      Property bit(s) to be added to these currently set.
     */
    void setProperty(uint16_t propertyValue)
    {
        m_properties |= propertyValue;
    }

    /**
     * Clears property.
     *
     * @param propertyValue
     *      Property bit(s) to be removed from these currently set.
     */
    void clearProperty(uint16_t propertyValue)
    {
        m_properties &= ~propertyValue;
    }

    /** Charset mapping to use. */
    CharsetMapping m_charsetMapping;

    /** Boxed mode flag. */
    const bool m_boxedMode;

    /** State - currently in box flag. */
    bool m_inBox;

    /** State - current in "hold mosaics* mode flag. */
    bool m_holdMosaics;

    /** State - stored mosaic character. */
    uint16_t m_holdMosaicCharacter;

    /** State - current charset. */
    Charset m_charset;

    /** State - current character mode. */
    CharsetMode m_charsetMode;

    /** State - current mosaic mode. */
    MosaicMode m_mosaicMode;

    /** State - current size mode. */
    SizeMode m_sizeMode;

    /** Value - current character. */
    uint16_t m_character;

    /** Value - current background index. */
    uint8_t m_bgColorIndex;

    /** Value - current foreground index. */
    uint8_t m_fgColorIndex;

    /** Value - current properties. */
    uint16_t m_properties;
};

} // namespace ttxdecoder

#endif /*TTXDECODER_PARSERROWCONTEXT_HPP_*/
