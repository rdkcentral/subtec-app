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


#ifndef FTCPP_FREETYPE_HPP_
#define FTCPP_FREETYPE_HPP_

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include <sstream>

#include "Exception.hpp"
#include "Library.hpp"

namespace ftcpp
{

/**
 * Freetype entry point.
 */
class Freetype
{
public:
    /**
     * Opens instance of freetype library.
     *
     * @return
     *      Opened library.
     *
     * @throw Exception
     *      on error.
     */
    static std::unique_ptr<Library> open()
    {
        FT_Library library = nullptr;

        auto error = FT_Init_FreeType(&library);
        if (error)
        {
            throw Exception("Initialization of Free Type library failed!", error);
        }

        return std::unique_ptr<Library>(new Library(library));
    }

private:
    Freetype() = delete;
};

} // namespace ftcpp

#endif /*FTCPP_FREETYPE_HPP_*/
