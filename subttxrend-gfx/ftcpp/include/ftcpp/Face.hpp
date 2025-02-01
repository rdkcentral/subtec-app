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


#ifndef FTCPP_FACE_HPP_
#define FTCPP_FACE_HPP_

#include <ft2build.h>
#include FT_FREETYPE_H
#include <sstream>

#include "Exception.hpp"

namespace ftcpp
{

/**
 * Freetype FT_Face wrapper.
 */
class Face
{
    // to allow access to constructor
    friend class Library;

    // not copyable
    Face(const Face&) = delete;
    Face& operator =(const Face&) = delete;

public:
    /**
     * Destructor.
     */
    ~Face()
    {
        FT_Done_Face(m_face);
    }

    /**
     * Returns native freetype object.
     *
     * @return
     *      Native freetype object.
     */
    FT_Face getNativeObject() const
    {
        return m_face;
    }

    /**
     * Returns current active size for this face.
     *
     * @return
     *      The current active size for this face.
     */
    FT_Size& getSize()
    {
        return m_face->size;
    }

    /**
     * Returns current active size for this face.
     *
     * @return
     *      The current active size for this face.
     */
    const FT_Size& getSize() const
    {
        return m_face->size;
    }

    /**
     * Returns the face's associated glyph slot(s).
     *
     * @return
     *      Associated glyph slot(s).
     */
    FT_GlyphSlot& getGlyph()
    {
        return m_face->glyph;
    }

    /**
     * Returns the face's associated glyph slot(s).
     *
     * @return
     *      Associated glyph slot(s).
     */
    const FT_GlyphSlot& getGlyph() const
    {
        return m_face->glyph;
    }

    /**
     * Requests the nominal size.
     *
     * See freetype manual for more details about the parameters.
     *
     * @param charWidth
     *      Width in 26.6 points.
     * @param charHeight
     *      Height in 26.6 points.
     * @param horzResolution
     *      Horizontal resolution.
     * @param vertResolution
     *      Vertical resolution.
     *
     * @throw Exception
     *      on error.
     */
    void setCharSize(FT_F26Dot6 charWidth,
                     FT_F26Dot6 charHeight,
                     FT_UInt horzResolution,
                     FT_UInt vertResolution)
    {
        auto error = FT_Set_Char_Size(getNativeObject(), charWidth, charHeight,
                horzResolution, vertResolution);
        if (error)
        {
            throw Exception("Setting char size failed!", error);
        }
    }

    /**
     * Requests the nominal size (in pixels).
     *
     * See freetype manual for more details about the parameters.
     *
     * @param pixelWidth
     *      Width in pixels.
     * @param pixelHeight
     *      Height in pixels.
     *
     * @throw Exception
     *      on error.
     */
    void setPixelSizes(FT_UInt pixelWidth,
                       FT_UInt pixelHeight)
    {
        auto error = FT_Set_Pixel_Sizes(getNativeObject(), pixelWidth,
                pixelHeight);
        if (error)
        {
            throw Exception("Setting pixel sizes failed!", error);
        }
    }

    /**
     * Load a single character glyph into the glyph slot
     *
     * @param charCode
     *      The glyph's character code.
     * @param loadFlags
     *      A flag indicating what to load for this glyph.
     *
     * @throw Exception
     *      on error.
     */
    void loadChar(FT_ULong charCode,
                  FT_Int32 loadFlags)
    {
        auto error = FT_Load_Char(getNativeObject(), charCode, loadFlags);
        if (error)
        {
            throw Exception("Loading char failed!", error);
        }
    }

    /**
     * Load a single glyph into the glyph slot
     *
     * @param glyphIndex
     *      The glyph's index.
     * @param loadFlags
     *      A flag indicating what to load for this glyph.
     *
     * @throw Exception
     *      on error.
     */
    void loadGlyph(FT_UInt glyphIndex,
                   FT_Int32 loadFlags)
    {
        auto error = FT_Load_Glyph(getNativeObject(), glyphIndex, loadFlags);
        if (error)
        {
            throw Exception("Loading glyph failed!", error);
        }
    }

private:
    /**
     * Constructor.
     *
     * @param face
     *      Wrapped freetype object.
     */
    Face(FT_Face face) :
            m_face(face)
    {
        // noop
    }

    /**  Wrapped freetype object. */
    FT_Face m_face;
};

} // namespace ftcpp

#endif /*FTCPP_FACE_HPP_*/
