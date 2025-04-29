/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/
#include "utils.h"

/**
 * @brief Extracts filename from given file path
 * 
 * This function takes a file path as input and returns the filename.
 * It handles both forward slashes ('/') and backslashes ('\')
 * as path separators.
 *
 * @param filePath Full file path
 * @return A std::string containing the filename. If no path separator is
 * found, the entire input string is considered the filename.
 */

std::string getFilename(const std::string& filePath) {
    // Find the last occurrence of '/' or '\' (depending on the OS)
    size_t lastSlash = filePath.find_last_of("/\\");

    if (lastSlash == std::string::npos) {
        // No slash found, so the entire path is the filename
        return filePath;
    } else {
        // Extract the filename from the path
        return filePath.substr(lastSlash + 1);
    }
}

/**
 * @brief Calls SendCCSetAttributePacket for 608cc and 708cc tests
 * 
 * This functions sets the given value for the attribute mentioned,
 * and calls SendCCSetAttributePacket.
 * 
 * @param type attribute type
 * @param value value to set to the attribute
 * @param channel ClosedCaptionsChannel to use to send packets
 */
void sendCCAttributePacket (short type, int value, ClosedCaptionsChannel *channel)
{
    int ccType = 0;
    using AttributesArray = std::array<uint32_t, 14> ;
    static constexpr AttributesArray masks = {
            GSW_CC_ATTRIB_FONT_COLOR,
            GSW_CC_ATTRIB_BACKGROUND_COLOR,
            GSW_CC_ATTRIB_FONT_OPACITY,
            GSW_CC_ATTRIB_BACKGROUND_OPACITY,
            GSW_CC_ATTRIB_FONT_STYLE,
            GSW_CC_ATTRIB_FONT_SIZE,
            GSW_CC_ATTRIB_FONT_ITALIC,
            GSW_CC_ATTRIB_FONT_UNDERLINE,
            GSW_CC_ATTRIB_BORDER_TYPE,
            GSW_CC_ATTRIB_BORDER_COLOR,
            GSW_CC_ATTRIB_WIN_COLOR,
            GSW_CC_ATTRIB_WIN_OPACITY,
            GSW_CC_ATTRIB_EDGE_TYPE,
            GSW_CC_ATTRIB_EDGE_COLOR
    };
    AttributesArray attributes{};

    // Find position of attribute type
    auto it = std::find(masks.begin(), masks.end(), type);

    // Check if the element was found
    if (it != masks.end()) {
        // Calculate the index using std::distance
        int index = std::distance(masks.begin(), it);
        attributes[index] = value;
    } else {
        debug_log (type << " - attribute type received, Invalid ! please check !");
    }

    channel->SendCCSetAttributePacket(ccType, uint32_t{type}, attributes);
}

/**
 * @brief Reset all cc attributes
 * 
 * This function is used to set all cc attributes to their defaults.
 * This is done in beginning of every attribute test to make sure tests run
 * independent of each other.
 * This function calls sendCCAttributePacket() for every attribute.
 * 
 * @param channel ClosedCaptionsChannel to use to send packets
 */
void resetAllCCAttributes(ClosedCaptionsChannel *channel)
{
    sendCCAttributePacket(GSW_CC_ATTRIB_FONT_COLOR, GSW_CC_EMBEDDED_COLOR, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_BACKGROUND_COLOR, GSW_CC_EMBEDDED_COLOR, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_FONT_OPACITY, -1, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_BACKGROUND_OPACITY, -1, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_FONT_STYLE, -1, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_FONT_SIZE, GSW_CC_FONT_SIZE_EMBEDDED, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_FONT_ITALIC, -1, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_FONT_UNDERLINE, -1, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_BORDER_TYPE, -1, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_BORDER_COLOR, GSW_CC_EMBEDDED_COLOR, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_WIN_COLOR, GSW_CC_EMBEDDED_COLOR, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_WIN_OPACITY, -1, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_EDGE_TYPE, -1, channel);
    sendCCAttributePacket(GSW_CC_ATTRIB_EDGE_COLOR, GSW_CC_EMBEDDED_COLOR, channel);
}

/**
 * @brief Reset all webvtt attributes
 * 
 * This function is used to set all webvtt attributes to their defaults.
 * This is done in beginning of every attribute test to make sure tests run
 * independent of each other.
 * This function calls SendCCSetAttributePacket() with all attributes set
 * to default.
 * 
 * @param channel WebVttChannel to use to send packets
 */
void resetAllWebVttAttributes(WebVttChannel *channel)
{
    std::uint32_t attributesMask = 0;
    attributesType attributesValues = {0};
    int ccType = 0;            /* Value not used by WebVTT */

    channel->SendTimestampPacket(0);

    attributesMask |= (1 << FONT_COLOR_ARR_POSITION);
    attributesValues[FONT_COLOR_ARR_POSITION] = COLOR_WHITE;
    attributesMask |= (1 << BACKGROUND_COLOR_ARR_POSITION);
    attributesValues[BACKGROUND_COLOR_ARR_POSITION] = COLOR_EMBEDDED;
    attributesMask |= (1 << FONT_OPACITY_ARR_POSITION);
    attributesValues[FONT_OPACITY_ARR_POSITION] = OPACITY_EMBEDDED;
    attributesMask |= (1 << BACKGROUND_OPACITY_ARR_POSITION);
    attributesValues[BACKGROUND_OPACITY_ARR_POSITION] = OPACITY_EMBEDDED;
    attributesMask |= (1 << FONT_STYLE_ARR_POSITION);
    attributesValues[FONT_STYLE_ARR_POSITION] = FONT_STYLE_DEFAULT;
    attributesMask |= (1 << FONT_SIZE_ARR_POSITION);
    attributesValues[FONT_SIZE_ARR_POSITION] = FONT_SIZE_EMBEDDED;
    attributesMask |= (1 << FONT_ITALIC_ARR_POSITION);
    attributesValues[FONT_ITALIC_ARR_POSITION] = 0;
    attributesMask |= (1 << FONT_UNDERLINE_ARR_POSITION);
    attributesValues[FONT_UNDERLINE_ARR_POSITION] = 0;
    attributesMask |= (1 << BORDER_TYPE_ARR_POSITION);
    attributesValues[BORDER_TYPE_ARR_POSITION] = EDGE_TYPE_NONE;
    attributesMask |= (1 << BORDER_COLOR_ARR_POSITION);
    attributesValues[BORDER_COLOR_ARR_POSITION] = COLOR_EMBEDDED;
    attributesMask |= (1 << WIN_COLOR_ARR_POSITION);
    attributesValues[WIN_COLOR_ARR_POSITION] = COLOR_EMBEDDED;
    attributesMask |= (1 << WIN_OPACITY_ARR_POSITION);
    attributesValues[WIN_OPACITY_ARR_POSITION] = OPACITY_EMBEDDED;
    attributesMask |= (1 << EDGE_TYPE_ARR_POSITION);
    attributesValues[EDGE_TYPE_ARR_POSITION] = EDGE_TYPE_NONE;
    attributesMask |= (1 << EDGE_COLOR_ARR_POSITION);
    attributesValues[EDGE_COLOR_ARR_POSITION] = COLOR_EMBEDDED;

    if (attributesMask)
    {
        channel->SendCCSetAttributePacket(ccType, attributesMask, attributesValues);
    }
}

/**
 * @brief Send webvtt attribute packet and data packet
 * 
 * This function sends webvtt attribute packet with attribute set to given
 * value. Also sends webvtt test file over SendDataPacket().
 * 
 * @param value value to set to attribute
 * @param filename webvtt test file name with path relative to test script
 * @param attribute attribute to be updated
 * @param channel WebVttChannel to use to send packets
 */
void sendCCAttributeWebvtt(int value, std::string filename, AttribPosInArray attribute, WebVttChannel *channel )
{
    sendWebVttAttributePacket (value, filename, attribute, channel);
    read_file_send_data_packet_webvtt (filename, channel);
}

/**
 * @brief Send webvtt attribute packet
 * 
 * This function sends webvtt attribute packet with attribute set to given
 * value.
 * 
 * @param value value to set to attribute
 * @param filename webvtt test file name with path relative to test script
 * @param attribute attribute to be updated
 * @param channel WebVttChannel to use to send packets
 */
void sendWebVttAttributePacket (int value, std::string filename, AttribPosInArray attribute, WebVttChannel *channel)
{
    std::uint32_t attributesMask = 0;
    attributesType attributesValues = {0};
    int ccType = 0;            /* Value not used by WebVTT */

    channel->SendTimestampPacket(0);

    attributesMask |= (1 << attribute);
    attributesValues[attribute] = value;
    if (attributesMask)
    {
        channel->SendCCSetAttributePacket(ccType, attributesMask, attributesValues);
    }
}

/**
 * @brief Read and send .vtt test file as data packet
 * 
 * This function sends webvtt test file using SendDataPacket().
 * 
 * @param filename webvtt test file name with path relative to test script
 * @param channel WebVttChannel to use to send packets
 */
void read_file_send_data_packet_webvtt(std::string filename, WebVttChannel *channel)
{
    std::vector<uint8_t> data;
    std::ifstream ifile = std::ifstream(filename, std::ios::in | std::ios::binary);
    
    if (ifile.fail())
    {
        debug_log("Input file " << filename << " can not be opened/does not exist. Aborting !");
        abort();
    }

    std::for_each(std::istreambuf_iterator<char>(ifile),
                std::istreambuf_iterator<char>(),
                [&data](const char c) {
                data.push_back(c);
                });

    channel->SendDataPacket(std::move(data));
}

/**
 * @brief Read and send .ttml test file as data packet
 * 
 * This function sends ttml test file using SendDataPacket().
 * 
 * @param filename ttml test file name with path relative to test script
 * @param channel TtmlChannel to use to send packets
 */
void read_file_send_data_packet_ttml(std::string filename, TtmlChannel *channel)
{
    std::vector<uint8_t> data;
    std::ifstream ifile = std::ifstream(filename, std::ios::in | std::ios::binary);
    
    if (ifile.fail())
    {
        debug_log("Input file " << filename << " can not be opened/does not exist. Aborting !");
        abort();
    }
    channel->SendTimestampPacket(0);

    std::for_each(std::istreambuf_iterator<char>(ifile),
                std::istreambuf_iterator<char>(),
                [&data](const char c) {
                data.push_back(c);
                });

    channel->SendDataPacket(std::move(data));
}

