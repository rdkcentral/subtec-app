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


#ifndef FTCPP_LIBRARY_HPP_
#define FTCPP_LIBRARY_HPP_

#include <ft2build.h>
#include FT_FREETYPE_H
#include <sstream>

#include "Face.hpp"
#include "Bitmap.hpp"

namespace ftcpp
{

/**
 * Freetype FT_Library wrapper.
 */
class Library
{
    // access to constructor
    friend class Freetype;

    // not copyable
    Library(const Library&) = delete;
    Library& operator =(const Library&) = delete;

public:
    /**
     * Destructor.
     */
    ~Library()
    {
        FT_Done_FreeType(m_library);
    }

    /**
     * Returns native freetype object.
     *
     * @return
     *      Native freetype object.
     */
    FT_Library getNativeObject() const
    {
        return m_library;
    }

    /**
     * Opens font by its pathname.
     *
     * @param filePathName
     *      Path to the font file.
     * @param faceIndex
     *      Index of the face to load (0 is always safe).
     *
     * @return
     *      Opened font face.
     *
     * @throw Exception
     *      on error.
     */
    std::unique_ptr<Face> newFace(const char* filePathName,
                                  FT_Long faceIndex)
    {
        FT_Face face = nullptr;

        auto error = FT_New_Face(getNativeObject(), filePathName, faceIndex,
                &face);
        if (error)
        {
            throw Exception("Creation of new face failed!", error);
        }

        return std::unique_ptr<Face>(new Face(face));
    }

    /**
     * Creates new bitmap.
     *
     * @return
     *      Created bitmap.
     */
    std::unique_ptr<Bitmap> newBitmap()
    {
        return std::unique_ptr<Bitmap>(new Bitmap(m_library));
    }

private:
    /**
     * Freetype.
     *
     * @param library
     *      Handle to opened freetype library instance.
     */
    Library(FT_Library library) :
            m_library(library)
    {
        // noop
    }

    /** Native freetype object. */
    FT_Library m_library;
};

} // namespace ftcpp

#endif /*FTCPP_LIBRARY_HPP_*/
