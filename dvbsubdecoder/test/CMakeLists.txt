##############################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2021 Liberty Global Service B.V.#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##############################################################################

project(dvbsubdecoder-test)

include(CTest)

cmake_minimum_required (VERSION 3.2)

#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/")

#
# Setup for coverage
#
include(CodeCoverage)
if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_C_FLAGS               "${CMAKE_C_FLAGS_COVERAGE} -fno-inline -fno-inline-small-functions -fno-default-inline")
set(CMAKE_CXX_FLAGS             "${CMAKE_CXX_FLAGS_COVERAGE} -fno-inline -fno-inline-small-functions -fno-default-inline")
set(CMAKE_EXE_LINKER_FLAGS      "${CMAKE_EXE_LINKER_FLAGS_COVERAGE}")
set(CMAKE_SHARED_LINKER_FLAGS   "${CMAKE_SHARED_LINKER_FLAGS_COVERAGE}")
endif(CMAKE_COMPILER_IS_GNUCXX)

#
# Packages to use
#
find_package(LibCppUnit REQUIRED)
find_package(LibSubTtxRendCommon REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/dvbsubdecoder)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)
include_directories(${LIBCPPUNIT_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})

#
# Macros
#
macro (add_cppunit_test _name)
    # invoke built-in add_executable
    add_executable(${ARGV})

    set_property(TARGET ${_name} PROPERTY CXX_STANDARD 11)

    target_link_libraries(${_name} ${LIBCPPUNIT_LIBRARIES})
if(CMAKE_COMPILER_IS_GNUCXX)
    target_link_libraries(${_name} gcov)
endif(CMAKE_COMPILER_IS_GNUCXX)

#SETUP_TARGET_FOR_COVERAGE(
    add_test(NAME ${_name} COMMAND ${_name} )
endmacro()

#
# Data files
#
# configure_file("test1.ini" "test1.ini" COPYONLY)

#
# Tests
#
add_cppunit_test(BitStream_Test
                 BitStream/BitStream_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/PesPacketReader.cpp
)

add_cppunit_test(Clut_Test
                 Clut/Clut_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/Clut.cpp
)

add_cppunit_test(Pixmap_Test
                 Pixmap/Pixmap_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/Pixmap.cpp
)

add_cppunit_test(PixelWriter_Test
                 PixelWriter/PixelWriter_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/PixelWriter.cpp
                 ../src/Pixmap.cpp
)

add_cppunit_test(ObjectParser_Test
                 ObjectParser/ObjectParser_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/ObjectParser.cpp
                 ../src/PixelWriter.cpp
                 ../src/Pixmap.cpp
                 ../src/PesPacketReader.cpp
)

add_cppunit_test(Exception_Test
                 Exception/Exception_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
)

add_cppunit_test(PesPacketReader_Test
                 PesPacketReader/PesPacketReader_test.cpp
                 common/TestRunner.cpp
                 ../src/PesPacketReader.cpp
)

add_cppunit_test(BasicAllocator_Test
                 BasicAllocator/BasicAllocator_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/BasicAllocator.cpp
)

add_cppunit_test(Array_Test
                 Array/Array_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
)

add_cppunit_test(Status_Test
                 Status/Status_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/Status.cpp
)

add_cppunit_test(Display_Test
                 Display/Display_test.cpp
                 common/TestRunner.cpp
)

add_cppunit_test(Page_Test
                 Page/Page_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
)

add_cppunit_test(ParserPCS_Test
                 ParserPCS/ParserPCS_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/ParserPCS.cpp
                 ../src/PesPacketReader.cpp
                 ../src/PixmapAllocator.cpp
                 ../src/Database.cpp
                 ../src/Status.cpp
                 ../src/Clut.cpp
                 ../src/Region.cpp
                 ../src/Pixmap.cpp
)
add_cppunit_test(RenderingState_Test
                 RenderingState/RenderingState_test.cpp
                 common/TestRunner.cpp
)

add_cppunit_test(PixmapAllocator_Test
                 PixmapAllocator/PixmapAllocator_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/PixmapAllocator.cpp
)

add_cppunit_test(ObjectPool_Test
                 ObjectPool/ObjectPool_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
)

add_cppunit_test(ObjectTablePool_Test
                 ObjectTablePool/ObjectTablePool_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
)

add_cppunit_test(PesBuffer_Test
                 PesBuffer/PesBuffer_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/PesBuffer.cpp
                 ../src/PesPacketReader.cpp
                 ../src/DynamicAllocator.cpp
)

add_cppunit_test(ObjectInstance_Test
                 ObjectInstance/ObjectInstance_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
)

add_cppunit_test(ParserEDS_Test
                 ParserEDS/ParserEDS_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/ParserEDS.cpp
                 ../src/PesPacketReader.cpp
                 ../src/PixmapAllocator.cpp
                 ../src/Database.cpp
                 ../src/Status.cpp
                 ../src/Clut.cpp
                 ../src/Region.cpp
                 ../src/Pixmap.cpp
)

add_cppunit_test(ParserDDS_Test
                 ParserDDS/ParserDDS_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/ParserDDS.cpp
                 ../src/PesPacketReader.cpp
                 ../src/PixmapAllocator.cpp
                 ../src/Database.cpp
                 ../src/Status.cpp
                 ../src/Clut.cpp
                 ../src/Region.cpp
                 ../src/Pixmap.cpp
)

add_cppunit_test(ParserODS_Test
                 ParserODS/ParserODS_test.cpp
                 ParserODS/ObjectParserStub.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/ParserODS.cpp
                 ../src/PesPacketReader.cpp
                 ../src/PixmapAllocator.cpp
                 ../src/Database.cpp
                 ../src/Status.cpp
                 ../src/Clut.cpp
                 ../src/Region.cpp
                 ../src/Pixmap.cpp
                 ../src/PixelWriter.cpp
)

add_cppunit_test(ParserRCS_Test
                 ParserRCS/ParserRCS_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/ParserRCS.cpp
                 ../src/PesPacketReader.cpp
                 ../src/PixmapAllocator.cpp
                 ../src/Database.cpp
                 ../src/Status.cpp
                 ../src/Clut.cpp
                 ../src/Region.cpp
                 ../src/Pixmap.cpp
                 ../src/PixelWriter.cpp
)

add_cppunit_test(ParserCDS_Test
                 ParserCDS/ParserCDS_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/ParserCDS.cpp
                 ../src/PesPacketReader.cpp
                 ../src/PixmapAllocator.cpp
                 ../src/ColorCalculator.cpp
                 ../src/Database.cpp
                 ../src/Status.cpp
                 ../src/Clut.cpp
                 ../src/Region.cpp
                 ../src/Pixmap.cpp
                 ../src/PixelWriter.cpp
)

add_cppunit_test(Parser_Test
                 Parser/Parser_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/Parser.cpp
                  ../src/PesPacketReader.cpp
                  ../src/PixmapAllocator.cpp
                  ../src/Database.cpp
                  ../src/PesBuffer.cpp
                  ../src/Status.cpp
                  ../src/Clut.cpp
                  ../src/Region.cpp
                  ../src/Pixmap.cpp
                  ../src/PixelWriter.cpp
                  ../src/DynamicAllocator.cpp
)

add_cppunit_test(Region_Test
                 Region/Region_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/Region.cpp
                 ../src/Pixmap.cpp
                 ../src/Clut.cpp
)

add_cppunit_test(Database_Test
                 Database/Database_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../src/Database.cpp
                 ../src/Region.cpp
                 ../src/Pixmap.cpp
                 ../src/Clut.cpp
                 ../src/PixmapAllocator.cpp
                 ../src/Status.cpp
)

add_cppunit_test(Decoder_Test
                 Decoder/Decoder_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 Decoder/TestCaseData.cpp
                 ../src/Clut.cpp
                 ../src/ColorCalculator.cpp
                 ../src/DynamicAllocator.cpp
                 ../src/DecoderFactory.cpp
                 ../src/DecoderImpl.cpp
                 ../src/Storage.cpp
                 ../src/Parser.cpp
                 ../src/ParserCDS.cpp
                 ../src/ParserDDS.cpp
                 ../src/ParserEDS.cpp
                 ../src/ParserODS.cpp
                 ../src/ParserPCS.cpp
                 ../src/ParserRCS.cpp
                 ../src/Database.cpp
                 ../src/ObjectParser.cpp
                 ../src/PesBuffer.cpp
                 ../src/PesPacketReader.cpp
                 ../src/PixelWriter.cpp
                 ../src/Pixmap.cpp
                 ../src/PixmapAllocator.cpp
                 ../src/Presenter.cpp
                 ../src/Region.cpp
                 ../src/Status.cpp
                 ../src/Storage.cpp
                 )

add_cppunit_test(ColorCalculator_Test
                 ColorCalculator/ColorCalculator_test.cpp
                 common/TestRunner.cpp
                 ../src/ColorCalculator.cpp
)

add_cppunit_test(Coverage_Test
                 Coverage/Coverage_test.cpp
                 common/TestRunner.cpp
                 common/Logger.cpp
                 ../include/dvbsubdecoder/Allocator.hpp
                 ../include/dvbsubdecoder/BasicAllocator.hpp
                 ../include/dvbsubdecoder/Decoder.hpp
                 ../include/dvbsubdecoder/DecoderClient.hpp
                 ../include/dvbsubdecoder/DecoderFactory.hpp
                 ../include/dvbsubdecoder/DynamicAllocator.hpp
                 ../include/dvbsubdecoder/Types.hpp
                 ../src/AllocatorTraits.hpp
                 ../src/Array.hpp
                 ../src/BasicAllocator.cpp
                 ../src/BitStream.hpp
                 ../src/Clut.cpp
                 ../src/Clut.hpp
                 ../src/ColorCalculator.cpp
                 ../src/ColorCalculator.hpp
                 ../src/Consts.hpp
                 ../src/Database.cpp
                 ../src/Database.hpp
                 ../src/DecoderFactory.cpp
                 ../src/DecoderImpl.cpp
                 ../src/DecoderImpl.hpp
                 ../src/Display.hpp
                 ../src/DynamicAllocator.cpp
                 ../src/ObjectInstance.hpp
                 ../src/ObjectParser.cpp
                 ../src/ObjectParser.hpp
                 ../src/ObjectPool.hpp
                 ../src/Page.hpp
                 ../src/Parser.cpp
                 ../src/Parser.hpp
                 ../src/ParserCDS.cpp
                 ../src/ParserCDS.hpp
                 ../src/ParserDDS.cpp
                 ../src/ParserDDS.hpp
                 ../src/ParserEDS.cpp
                 ../src/ParserEDS.hpp
                 ../src/ParserException.hpp
                 ../src/ParserODS.cpp
                 ../src/ParserODS.hpp
                 ../src/ParserPCS.cpp
                 ../src/ParserPCS.hpp
                 ../src/ParserRCS.cpp
                 ../src/ParserRCS.hpp
                 ../src/PesBuffer.cpp
                 ../src/PesBuffer.hpp
                 ../src/PesPacketReader.cpp
                 ../src/PesPacketReader.hpp
                 ../src/PixelWriter.cpp
                 ../src/PixelWriter.hpp
                 ../src/Pixmap.cpp
                 ../src/Pixmap.hpp
                 ../src/PixmapAllocator.cpp
                 ../src/PixmapAllocator.hpp
                 ../src/Presenter.cpp
                 ../src/Presenter.hpp
                 ../src/Region.cpp
                 ../src/Region.hpp
                 ../src/RenderingState.hpp
                 ../src/Status.cpp
                 ../src/Status.hpp
                 ../src/Storage.cpp
                 ../src/Storage.hpp
)

if(CMAKE_COMPILER_IS_GNUCXX)
setup_target_for_coverage(coverage ctest coverage)
endif(CMAKE_COMPILER_IS_GNUCXX)
