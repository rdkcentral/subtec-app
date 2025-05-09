/*****************************************************************************
* Copyright © 2018 Liberty Global B.V. and its Licensors.
* All rights reserved.
* Licensed by RDK Management, LLC under the terms of the RDK license.
* ============================================================================
* Liberty Global B.V. CONFIDENTIAL AND PROPRIETARY
* ============================================================================
* This file (and its contents) are the intellectual property of Liberty Global B.V.
* It may not be used, copied, distributed or otherwise disclosed in whole or in
* part without the express written permission of Liberty Global B.V.
* The RDK License agreement constitutes express written consent by Liberty Global.
* ============================================================================
* This software is the confidential and proprietary information of Liberty Global B.V.
* ("Confidential Information"). You shall not disclose this source code or
* such Confidential Information and shall use it only in accordance with the
* terms of the license agreement you entered into.
*
* LIBERTY GLOBAL B.V. MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
* SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE, OR NON-INFRINGEMENT. LIBERTY GLOBAL B.V. SHALL NOT BE LIABLE FOR
* ANY DAMAGES SUFFERED BY LICENSEE NOR SHALL THEY BE RESPONSIBLE AS A RESULT
* OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
******************************************************************************/

#include <subttxrend/common/Logger.hpp>
#include <subttxrend/common/ConfigProvider.hpp>

#include "DataDumper.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <memory>

#include <sys/stat.h>

namespace subttxrend
{
namespace ttmlengine
{

namespace
{
common::Logger g_logger("TtmlEngine", "DataDumper");

}

void DataDumper::setup(const common::ConfigProvider* provider)
{
    assert(provider);

    dumpTolog = (provider->getInt("DUMP_TO_LOG", 0) != 0);

    std::string dumpDir = provider->get("DUMP_DIR");
    if (dumpDir != "")
    {
        mkdir(dumpDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        dirPath.assign(dumpDir);
        dumpToFile = true;
        g_logger.info("ttml dump directory created: %s", dirPath.c_str());
    }
}

void DataDumper::dump(const std::uint8_t* buffer,
          std::size_t bufferSize) const
{
    if (dumpTolog)
    {
        toLog(buffer, bufferSize);
    }

    if (dumpToFile)
    {
        dumpXml(buffer, bufferSize);
    }
}

void DataDumper::toFile(const std::string filename,
            const std::uint8_t* buffer,
            std::size_t bufferLen) const
{
    if (! dirPath.empty())
    {
        g_logger.info("dumping: %p (size %zu) to %s", buffer, bufferLen, filename.c_str());

        std::string filePath = dirPath + filename;
        FILE* dumpFile = fopen(filePath.c_str(), "aw+");
        if (dumpFile)
        {
            auto written = fwrite(buffer, 1, bufferLen, dumpFile);
            g_logger.debug("written: %zu out of %zu to %s", written, bufferLen, filePath.c_str());
            fclose(dumpFile);
        }
        else
        {
            g_logger.error("could not open dump file: %s errno: %d", filePath.c_str(), errno);
        }
    }
}

bool DataDumper::imageDumpEnabled() const
{
   return dumpToFile;
}

std::vector<std::uint8_t> DataDumper::readTtmlFromFile(const std::string& path)
{
   std::vector<std::uint8_t> data;

   FILE* dataFile = fopen(path.c_str(), "r");
   if (dataFile)
   {
       fseek(dataFile,0,SEEK_END);
       auto fileSize = ftell(dataFile);
       fseek(dataFile,0,SEEK_SET);

       data.resize(fileSize);
       auto dataRead = fread(data.data(), 1, data.size(), dataFile);

       g_logger.info("read: %zu out of %zu from %s", dataRead, data.size(), path.c_str());

       fclose(dataFile);
   }
   else
   {
       g_logger.error("could not open dump file: %s errno: %d", path.c_str(), errno);
   }
   return data;
}

void DataDumper::dumpXml(const std::uint8_t* buffer,
             std::size_t bufferSize) const
{
    using namespace std::chrono;

    // get current time and get the number of milliseconds for the current second
    // = remainder after division into seconds
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // convert current time to std::time_t in order to convert to std::tm
    auto converttime = system_clock::to_time_t(now);
    std::tm tmtime = *std::localtime(&converttime);

    char hoursMinutesSeconds[24]{};

    // convert to HH:MM:SS string,
    (void)std::strftime(hoursMinutesSeconds, sizeof(hoursMinutesSeconds), "%T", &tmtime);

    std::ostringstream oss;

    oss << hoursMinutesSeconds;
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    auto filename = oss.str();

    std::replace(filename.begin(), filename.end(), ':', '_');
    std::replace(filename.begin(), filename.end(), '.', '_');

    filename += ".xml";

    toFile(filename, buffer, bufferSize);
}

void DataDumper::toLog(const std::uint8_t* buffer, std::size_t bufferSize) const
{
    g_logger.info("TTML DATA DUMP >>>>>>>>>>>>>>>>>>>>>>>>>");
    std::string str;
    str.resize(bufferSize);
    for (std::size_t i = 0; i < bufferSize; ++i )
    {
        if (buffer[i] == '\n')
        {
            g_logger.info("%s", str.c_str());
            str.clear();
        }
        else
        {
            str += (reinterpret_cast<const char*>(buffer)[i]);
        }
    }
    if (!str.empty())
    {
        g_logger.info("%s", str.c_str());
    }
    g_logger.info("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
}

} // namespace ttmlengine
} // namespace subttxrend

