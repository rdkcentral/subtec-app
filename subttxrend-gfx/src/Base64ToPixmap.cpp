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

#define PNG_DEBUG 3
#include <png.h>

#include "Base64ToPixmap.hpp"

#include <cstring>
#include <csetjmp>
#include <memory>
#include <cassert>

#include <subttxrend/common/Logger.hpp>
#include <glib.h>
#include <glib/gi18n.h>

namespace subttxrend
{
namespace gfx
{

namespace
{

common::Logger g_logger("Gfx", "Base64ToPixmap");

struct png_raw_data
{
    unsigned char *data;
    size_t ptr;
    size_t datalen;
};

void png_warning_handler(png_structp /*png_ptr*/, png_const_charp message)
{
   g_logger.oswarning("libpng warning: ", message);
}

void png_error_handler(png_structp png_ptr, png_const_charp message)
{
    g_logger.oserror("libpng error: ", message);

    jmp_buf* jmpbuf_ptr = static_cast<jmp_buf*>(png_get_error_ptr(png_ptr));

    assert(jmpbuf_ptr);
    longjmp(*jmpbuf_ptr, 1);
}

void read_function(png_structp png_ptr, png_bytep data, png_size_t length)
{
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    png_raw_data *raw_data = static_cast<png_raw_data *>(io_ptr);
    if (raw_data->ptr + length <= raw_data->datalen)
    {
        memcpy(data, &raw_data->data[raw_data->ptr], length);
        raw_data->ptr += length;
    }
    else
    {
        memset(data, 0, length);
        png_error_handler(png_ptr, "Reading error");
    }
}

std::unique_ptr<gfx::Bitmap> read_png(unsigned char *pngdata, const size_t pngdatalen)
{
    std::unique_ptr<gfx::Bitmap> ret;
    png_structp png_ptr = 0;
    jmp_buf jmpbuf;
    png_voidp jmpbuf_ptr = &jmpbuf;
    struct png_structs_t {
        png_structp png_ptr;
        png_infop info_ptr;
    } png_structs;
    png_infop info_ptr = 0;
    png_raw_data raw_data;
    png_uint_32 width = 0;
    png_uint_32 height = 0;
    int bitDepth = 0;
    int colorType = -1;
    png_uint_32 retval;
    png_uint_32 bytesPerRow;
    uint32_t rowIdx;

    if (png_sig_cmp(pngdata, 0, 8) != 0)
    {
        g_logger.warning("not png!");
        return ret;
    }

    // get PNG file info struct (memory is allocated by libpng)
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, jmpbuf_ptr, png_error_handler, png_warning_handler);
    if (png_ptr == 0) {
        g_logger.warning("couldn't png_create_read_struct");
        return ret;
    }

    // get PNG image data info struct (memory is allocated by libpng)
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == 0) {
        // libpng must free file info struct memory before we bail
        png_destroy_read_struct(&png_ptr, 0, 0);
        g_logger.warning("couldn't png_create_info_struct");
        return ret;
    }

    png_structs = {png_ptr, info_ptr};
    // on return png_cleanup_guard is deleted, unique_ptr will handle destroying png_ptr and info_ptr structs
    std::unique_ptr<png_structs_t,std::function<void(png_structs_t*)>> png_cleanup_guard = {&png_structs, [](png_structs_t* guarded){
        png_destroy_read_struct(&guarded->png_ptr, &guarded->info_ptr, nullptr);
    } } ;

    raw_data = { pngdata, 0, pngdatalen};

    // setjmp must be called each time after allocating memory and before any libpng function that can return an error
    if (setjmp(jmpbuf))
    {
        return std::unique_ptr<gfx::Bitmap>();
    }

    png_set_read_fn(png_ptr, &raw_data, read_function);
    png_read_info(png_ptr, info_ptr);

    retval = png_get_IHDR(png_ptr, info_ptr,
                                      &width,
                                      &height,
                                      &bitDepth,
                                      &colorType,
                                      0, 0, 0);

    if (retval != 1)
    {
        g_logger.warning("couldn't png_get_IHDR");
        return ret; // add error handling and cleanup
    }

    if (colorType == PNG_COLOR_TYPE_PALETTE) {
        // convert to rgba
        png_set_palette_to_rgb(png_ptr);
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        {
            png_set_tRNS_to_alpha(png_ptr);
        }
        else
        {
            g_logger.warning("no tRNS in palette image");
            return ret;
        }

        png_read_update_info(png_ptr, info_ptr);
    }

    if (colorType == PNG_COLOR_TYPE_PALETTE || colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
        bytesPerRow = png_get_rowbytes(png_ptr, info_ptr);
        ret = std::make_unique<gfx::Bitmap>(width, height, bytesPerRow);
        // setjmp must be called each time after allocating memory and before any libpng function that can return an error, e.g. here
        // if setjmp was not called and error handler called by png_read_row - we'd have memleaks from gfx::Bitmap allocation
        if (setjmp(jmpbuf))
        {
            return std::unique_ptr<gfx::Bitmap>();
        }
        // read single row at the time
        for (rowIdx = 0; rowIdx < height; ++rowIdx)
        {
            png_read_row(png_ptr, static_cast<png_bytep>(&ret->m_buffer[rowIdx * bytesPerRow]), nullptr);
        }
    } else {
        g_logger.warning("Invalid PNG ColorType enum value given.");
    }

    return ret;
}

bool areColorsEqual(const std::uint8_t color1[4], const std::uint8_t color2[4])
{
    return (color1[0] == color2[0])
            && (color1[1] == color2[1])
            && (color1[2] == color2[2])
            && (color1[3] == color2[3]);
}

/**
 * Funtion for detecting broken background color:
 *  - color is not fully transparent
 *  - color is the same in all corners
 *
 * @param bmp
 *      Source bitmap.
 * @param colorToDisable
 *      Output param - color to be replaced.
 * @return
 *      True if broken background color was deteced.
 */
bool findBgColorToDisable(std::unique_ptr<gfx::Bitmap>& bmp, std::uint8_t colorToDisable[4])
{
    const std::uint8_t* topLeft = &bmp->m_buffer[0];
    const std::uint8_t* topRight = &bmp->m_buffer[(bmp->m_width - 1) * 4];
    const std::uint8_t* bottomLeft = &bmp->m_buffer[((bmp->m_height - 1) * bmp->m_stride)];
    const std::uint8_t* bottomRight = &bmp->m_buffer[((bmp->m_height - 1) * bmp->m_stride) + (bmp->m_width - 1) * 4];

    static const std::uint8_t TRASPARENT_COLOR[4]{0, 0, 0, 0};

    // broken background color detection - not fully transparent and the same in all corners
    if( ! areColorsEqual(topLeft, TRASPARENT_COLOR)
            && areColorsEqual(topLeft, topRight)
            && areColorsEqual(topLeft, bottomLeft)
            && areColorsEqual(topLeft, bottomRight))
    {
        colorToDisable[0] = bmp->m_buffer[0];
        colorToDisable[1] = bmp->m_buffer[1];
        colorToDisable[2] = bmp->m_buffer[2];
        colorToDisable[3] = bmp->m_buffer[3];

        g_logger.warning("invalid background color detected: #%02x%02x%02x%02x, transparent will be used instead",
                       colorToDisable[0],
                       colorToDisable[1],
                       colorToDisable[2],
                       colorToDisable[3]);

        return true;

    }
    else
    {
        g_logger.debug("no invalid background color detected");
        return false;
    }
}

void convertToTargetFormat(std::unique_ptr<gfx::Bitmap>& bmp, bool applyAlpha)
{
    const uint32_t stride = bmp->m_stride;
    const uint32_t width = bmp->m_width;

    std::uint8_t colorToDisable[4]{};
    bool disableColor = findBgColorToDisable(bmp, colorToDisable);

    for (size_t y = 0 ; y < static_cast<size_t>(bmp->m_height); ++y)
    {
        for (size_t offset = y * stride; offset < y * stride + width * 4 ; offset += 4)
        {
            // workaround for HZN4PD-13329 [VOD] Screen turns darker when the subtitles appears on the screen
            // replace invalid color with fully transparent one
            if (disableColor && areColorsEqual(&bmp->m_buffer[offset], colorToDisable))
            {
                bmp->m_buffer[offset + 0] = 0;
                bmp->m_buffer[offset + 1] = 0;
                bmp->m_buffer[offset + 2] = 0;
                bmp->m_buffer[offset + 3] = 0;
            }
            else
            {
                std::uint8_t originalPixel[4] = { bmp->m_buffer[offset],
                        bmp->m_buffer[offset + 1],
                        bmp->m_buffer[offset + 2],
                        bmp->m_buffer[offset + 3]};

                const std::uint16_t a = applyAlpha ? (1 + originalPixel[3]) : 256;

                // NOTE: wayland uses ARGB in little endian format, libpng return RGBA in big endian, so we have
                // to convert: bytes R G B A returned from png bitmap have to be put in B G R A order
                bmp->m_buffer[offset + 0] = a * originalPixel[2] >> 8;
                bmp->m_buffer[offset + 1] = a * originalPixel[1] >> 8;
                bmp->m_buffer[offset + 2] = a * originalPixel[0] >> 8;
                bmp->m_buffer[offset + 3] = originalPixel[3];
            }
        }
    }
}

} // namespace anonymous


template<typename T, void(*Fn)(T*)> struct Deleter {
   inline void operator() (void *p) const
   {
      if (p)
         Fn(static_cast<T*>(p));
   }
};

std::unique_ptr<gfx::Bitmap> base64toPixmap(const std::string &base64txt,
                                            bool applyRGBAlphaPremult,
                                            PngCallback pngCallback)
{
    size_t outLen = 0;
    std::unique_ptr<uint8_t[], Deleter<void, g_free> > decoded = std::unique_ptr<uint8_t[], Deleter<void, g_free> >(g_base64_decode (base64txt.c_str(), &outLen));
    std::unique_ptr<gfx::Bitmap> ret;
    if (decoded.get() != NULL)
    {
        if (pngCallback)
        {
            pngCallback(decoded.get(), outLen);
        }
        ret = read_png(decoded.get(), outLen);
        if (ret)
        {
            convertToTargetFormat(ret, applyRGBAlphaPremult);
        }
    }
    return ret;
}

}   // namespace gfx
}   // namespace subttxrend


