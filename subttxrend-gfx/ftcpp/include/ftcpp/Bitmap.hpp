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


#ifndef FTCPP_BITMAP_HPP_
#define FTCPP_BITMAP_HPP_

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H

#include "Exception.hpp"

namespace ftcpp
{

/**
 * Freetype FT_Bitmap wrapper.
 */
class Bitmap
{
    // to allow access to constructor
    friend class Library;

    // not copyable
    Bitmap(const Bitmap&) = delete;
    Bitmap& operator =(const Bitmap&) = delete;

public:
    /**
     * Destructor.
     */
    ~Bitmap()
    {
        FT_Bitmap_Done(m_library, getNativeObject());
    }

    /**
     * Returns native freetype object.
     *
     * @return
     *      Native freetype object.
     */
    FT_Bitmap* getNativeObject()
    {
        return &m_bitmap;
    }

    /**
     * Operator ->.
     *
     * @return
     *      Wrapped object pointer.
     */
    FT_Bitmap* operator ->()
    {
        return &m_bitmap;
    }

    /**
     * Operator ->.
     *
     * @return
     *      Wrapped object pointer.
     */
    const FT_Bitmap* operator ->() const
    {
        return &m_bitmap;
    }

    /**
     * Converts the given bitmap to grayscale and store it in this bitmap.
     *
     * @param source
     *      Source bitmap.
     */
    void convert(const FT_Bitmap& source)
    {
        auto error = FT_Bitmap_Convert(m_library, &source, &m_bitmap, 1);
        if (error)
        {
            throw Exception("Bitmap convertion failed!", error);
        }
    }

private:
    /**
     * Constructor.
     *
     * @param library
     *      Library handle.
     */
    Bitmap(FT_Library library) :
            m_library(library)
    {
        FT_Bitmap_Init(&m_bitmap);
    }

    /** Library handle. */
    FT_Library m_library;

    /**  Wrapped freetype object. */
    FT_Bitmap m_bitmap;
};

} // namespace ftcpp

#endif /*FTCPP_BITMAP_HPP_*/
