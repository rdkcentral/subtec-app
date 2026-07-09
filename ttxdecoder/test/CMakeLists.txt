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

project(ttxdecoder-test)

enable_testing()

cmake_minimum_required (VERSION 3.2)

#
# Directory with modules
#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules/")

#
# Packages to use
#
find_package(LibCppUnit REQUIRED)
find_package(LibSubTtxRendCommon REQUIRED)

#
# Include directories
#
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include/ttxdecoder)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${LIBCPPUNIT_INCLUDE_DIRS})
include_directories(${LIBSUBTTXRENDCOMMON_INCLUDE_DIRS})

#
# Macros
#
macro (add_cppunit_test _name)
    # invoke built-in add_executable
    add_executable(${ARGV})

    set_property(TARGET ${_name} PROPERTY CXX_STANDARD 14)

    target_link_libraries(${_name} ${LIBCPPUNIT_LIBRARIES})


    add_test(NAME ${_name} COMMAND ${_name} )
endmacro()

#
# Tests
#
add_cppunit_test(CacheImpl_Test
                 ../src/CacheImpl.cpp
                 CacheImpl_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(CharsetMapping_Test
                 ../src/CharsetMapping.cpp
                 ../src/DefaultCharsets.cpp
                 CharsetMapping_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(Collector_Test
                 ../src/Collector.cpp
                 ../src/Hamming.cpp
                 ../src/PesPacketReader.cpp
                 Collector_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(Database_Test
                 ../src/Database.cpp
                 Database_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(DecodedPage_Test
                 ../src/DecodedPage.cpp
                 DecodedPage_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(Decoder_Test
                 ../src/Decoder.cpp
                 ../src/Collector.cpp
                 ../src/MetadataProcessor.cpp
                 ../src/TopNavProcessor.cpp
                 ../src/Database.cpp
                 ../src/PesPacketReader.cpp
                 ../src/Hamming.cpp
                 Decoder_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(EngineImpl_Test
                 ../src/EngineImpl.cpp
                 ../src/DecodedPage.cpp
                 ../src/Database.cpp
                 ../src/CacheImpl.cpp
                 ../src/PesBuffer.cpp
                 ../src/Decoder.cpp
                 ../src/Parser.cpp
                 ../src/ParserX26.cpp
                 ../src/Collector.cpp
                 ../src/MetadataProcessor.cpp
                 ../src/TopNavProcessor.cpp
                 ../src/PesPacketReader.cpp
                 ../src/Hamming.cpp
                 ../src/CharsetMapping.cpp
                 ../src/DefaultCharsets.cpp
                 EngineImpl_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(Hamming_Test
                 ../src/Hamming.cpp
                 Hamming_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(MetadataProcessor_Test
                 ../src/MetadataProcessor.cpp
                 ../src/Database.cpp
                 ../src/TopNavProcessor.cpp
                 MetadataProcessor_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(Parser_Test
                 ../src/Parser.cpp
                 ../src/ParserX26.cpp
                 ../src/Database.cpp
                 ../src/DecodedPage.cpp
                 ../src/CharsetMapping.cpp
                 ../src/DefaultCharsets.cpp
                 Parser_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(ParserX26_Test
                 ../src/Parser.cpp
                 ../src/ParserX26.cpp
                 ../src/Database.cpp
                 ../src/DecodedPage.cpp
                 ../src/CharsetMapping.cpp
                 ../src/DefaultCharsets.cpp
                 ParserX26_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(PesBuffer_Test
                 ../src/PesBuffer.cpp
                 ../src/PesPacketReader.cpp
                 PesBuffer_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(PesPacketReader_Test
                 ../src/PesPacketReader.cpp
                 PesPacketReader_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)

add_cppunit_test(TopNavProcessor_Test
                 ../src/TopNavProcessor.cpp
                 ../src/Database.cpp
                 TopNavProcessor_test.cpp
                 TestRunner.cpp
                 Logger.cpp
)
