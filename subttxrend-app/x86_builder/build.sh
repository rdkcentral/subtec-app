#!/bin/bash
##############################################################################
# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2021 Liberty Global Service B.V.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##############################################################################

# Main socket path when running the application - can be overridden in the environment
MSP=${MSP:-/tmp/pes_data_main}

# Get absolute name
#
# $1 - relative name
get_abs_filename() {
  echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
}

# Params
# $1 - source directory
# $2 - build directory
# $3 - install dir / find root path
# $4 - extra CMAKE params
function build_project_cmake {
    echo "Building project: $2"

    mkdir -p $2
    pushd $2

    cmake \
        -DCMAKE_INSTALL_PREFIX=$3/usr/local \
        -DCMAKE_MODULE_PATH=$3/usr/local/share/cmake/Modules \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_FIND_ROOT_PATH=$3 \
        -DCMAKE_INSTALL_NAME_DIR=$3/usr/local/lib \
        -DCMAKE_CXX_FLAGS="-DPC_BUILD" \
        $4 \
        $1
    make -k $MAKE_JOBS_OPT VERBOSE=1
    make install

    popd
}

# Params
# $1 - build directory
# $2 - source directory
# $n - subproject build directories
function build_doc_aggregate {

    echo "Building documentation: $1"

    mkdir -p $1
    pushd $1

    cp $2/* .
    for ARG in `seq 3 $#`
    do
        echo "TAGFILES += ${!ARG}/tagfile.tag=${!ARG}/html" >> Doxyfile
    done

    doxygen

    if [ -s warnings.txt ]
    then
        gedit warnings.txt &
    fi
    firefox html/index.html &

    popd
}

# Params
# $1 - build directory
function build_doc_cmake {
    echo "Building documentation: $1"

    pushd $1

    make doc
    if [ -s warnings.txt ]
    then
        gedit warnings.txt &
    fi

    popd
}

# Params
# $1 - source directory
# $2 - build directory
# $3 - install dir / find root path
# $4 - extra configure params
# $. - patches
function build_project_autotools {
    echo "Building library: $2"

    if [ ! -d "$2" ]; then
        mkdir -p $2
        pushd $2

        # copy the sources
        cp -a $1/. ./

        for ARG in `seq 5 $#`
        do
            echo "Patching: $ARG / $# => ${!ARG}"
            patch -p1 < ${!ARG}
        done

        chmod +x autogen.sh

        ./autogen.sh
        ./configure \
            CPPFLAGS="-I$3/usr/local/include" \
            --prefix=$3/usr/local \
            $4
    else
        pushd $2
    fi

    make -k $MAKE_JOBS_OPT V=1
    make $MAKE_JOBS_OPT install

    popd
}

# Params
# $1 - source directory
# $2 - build directory
# $3 - install dir / find root path
# $4 - extra CMAKE params
function build_tests {
    echo "Building tests: $2"

    mkdir -p $2
    pushd $2

    cmake -DCMAKE_INSTALL_PREFIX=$3/usr/local \
          -DCMAKE_MODULE_PATH=$3/usr/local/share/cmake/Modules \
          -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_FIND_ROOT_PATH=$3 \
          $4 \
          $1
    make $PARALLEL_JOBS_OPT

    if cat Makefile | egrep "\binstall\b" 2> /dev/null
    then
        make install
    fi
    popd
}

# Params
# $1 - build directory
function run_tests {
    pushd $1

    LD_LIBRARY_PATH=$INSTALL_DIR/usr/local/lib ctest --output-on-failure

    popd
}

# Params
# $1 - build directory
function run_tests_coverage {
    pushd $1

    make coverage
    firefox coverage/index.html

    popd
}

#============================================================================
#============================================================================

set -e

if [ "$#" -lt 1 ] ; then
    echo ""
    echo "---------------------------------------------"
    echo "Use: $0 <command>"
    echo "---------------------------------------------"
    echo "Example: $0 build"
    echo "---------------------------------------------"
    echo "Available commands:"
    echo "full:   clean + build + dotest + run"
    echo "clean:  clean everything"
    echo "build:  build everything"
    echo "run:    run the application"
    echo "fast:   build with -j8 (jobs) option"
    echo "dotest: build, build tests, run tests"
    echo "buildunits $1: build specified unit"
    echo "testunits $1: build, build tests, run tests for unit"
    echo "doc:    build, build docs, show docs warnings"
    echo "---------------------------------------------"
    echo ""
    exit
fi

THIS_DIR="$(realpath $(dirname $0))"
#OE_BUILD_DIR=$(realpath ${THIS_DIR}/../../../../../..)
OE_BUILD_DIR=$THIS_DIR/../..

COMMAND=$1
UNIT_NAMES=${@:2}
BASE_DIR=$(pwd)

echo ""
echo "---------------------------------------------"
echo "Configuration:"
echo "- OE_BUILD_DIR  = $OE_BUILD_DIR"
echo "- COMMAND       = $COMMAND"
echo "- RUN DIRECTORY = $BASE_DIR"
echo "---------------------------------------------"
echo ""

export PARALLEL_JOBS_OPT=-j8

if [ "$COMMAND" == "fast" ] ; then
export MAKE_JOBS_OPT=$PARALLEL_JOBS_OPT
COMMAND='build'
else
export MAKE_JOBS_OPT=
fi

ONEMW_ROOT_DIR=$OE_BUILD_DIR
AV_PROJECTS_DIR=$OE_BUILD_DIR
INSTALL_DIR=$OE_BUILD_DIR/subttxrend-app/x86_builder/build/install
RDK_COMPONENTS_DIR=$ONEMW_ROOT_DIR/components/generic/dvb/
RDK_PATCHES_DIR=$ONEMW_ROOT_DIR/meta-lgi-om-common/meta-rdk/recipes-extended/

#
# export path for .pc for pkg-config
#
if [[ "$OSTYPE" == "darwin"* ]]; then
export PKG_CONFIG_PATH+=$INSTALL_DIR/usr/lib/pkgconfig/:$INSTALL_DIR/usr/local/lib/pkgconfig/:/opt/homebrew/lib/pkgconfig
else
export PKG_CONFIG_PATH+=$INSTALL_DIR/usr/lib/pkgconfig/:$INSTALL_DIR/usr/local/lib/pkgconfig/
fi

if [ "$COMMAND" == "full" ] ; then

$0 clean
$0 fast
$0 dotest
$0 run

elif [ "$COMMAND" == "clean" ] ; then

rm -rf build

elif [ "$COMMAND" == "build" ] ; then

build_project_cmake     $BASE_DIR/src/rdklogger                     build/rdklogger                     $INSTALL_DIR    ""
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-common          build/subttxrend-common             $INSTALL_DIR    -DBUILD_RDK_REFERENCE=1
build_project_cmake     $AV_PROJECTS_DIR/ttxdecoder                 build/ttxdecoder                    $INSTALL_DIR    ""
build_project_cmake     $AV_PROJECTS_DIR/dvbsubdecoder              build/dvbsubdecoder                 $INSTALL_DIR    ""
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-protocol        build/subttxrend-protocol           $INSTALL_DIR    ""
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-socksrc         build/subttxrend-socksrc            $INSTALL_DIR    ""
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-dbus            build/subttxrend-dbus               $INSTALL_DIR    ""
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-gfx             build/subttxrend-gfx                $INSTALL_DIR    -DWITH_OPENGL=1
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-dvbsub          build/subttxrend-dvbsub             $INSTALL_DIR    ""
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-ttxt            build/subttxrend-ttxt               $INSTALL_DIR    ""
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-ttml            build/subttxrend-ttml               $INSTALL_DIR    -DBUILD_RDK_REFERENCE=1
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-webvtt          build/subttxrend-webvtt             $INSTALL_DIR    ""
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-scte            build/subttxrend-scte               $INSTALL_DIR    ""
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-cc              build/subttxrend-cc                 $INSTALL_DIR    ""
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-ctrl            build/subttxrend-ctrl               $INSTALL_DIR    ""
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-app             build/subttxrend-app                $INSTALL_DIR    -DINSTALL_CONFIG_FILE=OFF
build_project_cmake     $AV_PROJECTS_DIR/subttxrend-testapps        build/subttxrend-testapps           $INSTALL_DIR    ""

ls -Rl $INSTALL_DIR

elif [ "$COMMAND" == "buildunits" ] ; then

if [ "$#" -lt 2 ]
then
    echo "Need to pass in unit names"
    return 0
else
    for unit in $UNIT_NAMES
    do
        build_project_cmake     $AV_PROJECTS_DIR/$unit            build/$unit               $INSTALL_DIR    ""
    done
fi

elif [ "$COMMAND" == "testunits" ] ; then

if [ "$#" -lt 2 ]
then
    echo "Need to pass in unit names"
    return 0
else
    for unit in $UNIT_NAMES
    do
	echo "Building $unit"
        build_tests     $AV_PROJECTS_DIR/$unit/test    build/tests-$unit      $INSTALL_DIR
    done
    for test in $UNIT_NAMES
    do
	echo "Running tests for $test"
        run_tests       build/tests-$test
    done
fi

elif [ "$COMMAND" == "buildtest" ] ; then

# first - build the standard things
# $0 fast

# build the tests
build_tests     $AV_PROJECTS_DIR/subttxrend-common/test    build/tests-subttxrend-common      $INSTALL_DIR
build_tests     $AV_PROJECTS_DIR/subttxrend-protocol/test  build/tests-subttxrend-protocol    $INSTALL_DIR
build_tests     $AV_PROJECTS_DIR/dvbsubdecoder/test        build/tests-dvbsubdecoder          $INSTALL_DIR
build_tests     $AV_PROJECTS_DIR/subttxrend-ttml/test      build/tests-subttxrend-ttml        $INSTALL_DIR
build_tests     $AV_PROJECTS_DIR/subttxrend-webvtt/test      build/tests-subttxrend-webvtt        $INSTALL_DIR 1
build_tests     $AV_PROJECTS_DIR/ttxdecoder/test           build/tests-ttxdecoder             $INSTALL_DIR
#uild_tests     $AV_PROJECTS_DIR/subttxrend-cc/tests       build/tests-subttxrend-cc          $INSTALL_DIR

elif [ "$COMMAND" == "dotest" ] ; then

$0 buildtest

echo "#########################################################################"

# running the tests
run_tests       build/tests-subttxrend-common
run_tests       build/tests-subttxrend-protocol
run_tests       build/tests-dvbsubdecoder
run_tests       build/tests-subttxrend-ttml
run_tests       build/tests-subttxrend-webvtt
run_tests       build/tests-ttxdecoder
#run_tests       build/tests-subttxrend-cc

elif [ "$COMMAND" == "testcoverage" ] ; then

$0 $1 buildtest

run_tests_coverage  build/tests-dvbsubdecoder

elif [ "$COMMAND" == "run" ] ; then

$0 fast

LD_LIBRARY_PATH=$INSTALL_DIR/usr/local/lib $INSTALL_DIR/usr/local/bin/subttxrend-app -msp=$MSP -cfp=$THIS_DIR/config.ini

elif [ "$COMMAND" == "doc" ] ; then

# first - build the standard things
$0 fast

build_doc_cmake     build/subttxrend-app
build_doc_cmake     build/subttxrend-ctrl
build_doc_cmake     build/subttxrend-common
build_doc_cmake     build/subttxrend-dbus
build_doc_cmake     build/subttxrend-protocol
build_doc_cmake     build/subttxrend-socksrc
build_doc_cmake     build/subttxrend-gfx
build_doc_cmake     build/subttxrend-dvbsub
build_doc_cmake     build/subttxrend-ttxt
build_doc_cmake     build/subttxrend-ttml
build_doc_cmake     build/ttxdecoder
build_doc_cmake     build/dvbsubdecoder
build_doc_aggregate build/documentation $BASE_DIR/src/doc \
    $BASE_DIR/build/subttxrend-ctrl \
    $BASE_DIR/build/subttxrend-app \
    $BASE_DIR/build/subttxrend-common \
    $BASE_DIR/build/subttxrend-dbus \
    $BASE_DIR/build/subttxrend-protocol \
    $BASE_DIR/build/subttxrend-socksrc \
    $BASE_DIR/build/subttxrend-gfx \
    $BASE_DIR/build/subttxrend-dvbsub \
    $BASE_DIR/build/subttxrend-ttxt \
    $BASE_DIR/build/subttxrend-ttml \
    $BASE_DIR/build/ttxdecoder \
    $BASE_DIR/build/dvbsubdecoder


else

echo "Unkown command: $COMMAND"

fi
