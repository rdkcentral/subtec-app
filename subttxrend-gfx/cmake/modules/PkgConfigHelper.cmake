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

#
# CMAKE helper for resolving libraries using pkg-config
#

# Use the pkgconfig
find_package(PkgConfig REQUIRED)

# Include find package helpers
include(FindPackageHandleStandardArgs)

macro(pkgconfig_resolve _MODNAME _PKGNAME _INCFILE _LIBNAME)
    message(STATUS "Resolving pkg-config package for: ${_MODNAME} -> ${_PKGNAME}")
    
	string(TOUPPER ${_MODNAME} _PREFIX)

	# Find the component information
	pkg_check_modules(PCDATA_${_PREFIX} QUIET ${_PKGNAME})

	# <XPREFIX>_FOUND          - set to 1 if module(s) exist
	# <XPREFIX>_LIBRARIES      - only the libraries (w/o the '-l')
	# <XPREFIX>_LIBRARY_DIRS   - the paths of the libraries (w/o the '-L')
	# <XPREFIX>_LDFLAGS        - all required linker flags
	# <XPREFIX>_LDFLAGS_OTHER  - all other linker flags
	# <XPREFIX>_INCLUDE_DIRS   - the '-I' preprocessor flags (w/o the '-I')
	# <XPREFIX>_CFLAGS         - all required cflags
	# <XPREFIX>_CFLAGS_OTHER   - the other compiler flags
	
	# <XPREFIX>_VERSION    - version of the module
	# <XPREFIX>_PREFIX     - prefix-directory of the module
	# <XPREFIX>_INCLUDEDIR - include-dir of the module
	# <XPREFIX>_LIBDIR     - lib-dir of the module
	
	message(STATUS "PCDATA_${_PREFIX}_FOUND         = ${PCDATA_${_PREFIX}_FOUND}")
	message(STATUS "PCDATA_${_PREFIX}_LIBRARIES     = ${PCDATA_${_PREFIX}_LIBRARIES}")
	message(STATUS "PCDATA_${_PREFIX}_LIBRARY_DIRS  = ${PCDATA_${_PREFIX}_LIBRARY_DIRS}")
	message(STATUS "PCDATA_${_PREFIX}_LDFLAGS       = ${PCDATA_${_PREFIX}_LDFLAGS}")
	message(STATUS "PCDATA_${_PREFIX}_LDFLAGS_OTHER = ${PCDATA_${_PREFIX}_LDFLAGS_OTHER}")
	message(STATUS "PCDATA_${_PREFIX}_INCLUDE_DIRS  = ${PCDATA_${_PREFIX}_INCLUDE_DIRS}")
	message(STATUS "PCDATA_${_PREFIX}_CFLAGS        = ${PCDATA_${_PREFIX}_CFLAGS}")
	message(STATUS "PCDATA_${_PREFIX}_CFLAGS_OTHER  = ${PCDATA_${_PREFIX}_CFLAGS_OTHER}")
	message(STATUS "PCDATA_${_PREFIX}_VERSION       = ${PCDATA_${_PREFIX}_VERSION}")
	message(STATUS "PCDATA_${_PREFIX}_PREFIX        = ${PCDATA_${_PREFIX}_PREFIX}")
	message(STATUS "PCDATA_${_PREFIX}_INCLUDEDIR    = ${PCDATA_${_PREFIX}_INCLUDEDIR}")
	message(STATUS "PCDATA_${_PREFIX}_LIBDIR        = ${PCDATA_${_PREFIX}_LIBDIR}")
	
	find_path(LIBDATA_${_PREFIX}_INCLUDE_DIR
	          NAMES ${_INCFILE}
	          HINTS ${PCDATA_${_PREFIX}_INCLUDEDIR} ${PCDATA_${_PREFIX}_INCLUDE_DIRS}
    )
	
	find_library(LIBDATA_${_PREFIX}_LIBRARY
	             NAMES ${_LIBNAME}
	             HINTS ${PCDATA_${_PREFIX}_LIBDIR} ${PCDATA_${_PREFIX}_LIBRARY_DIRS}
    )
	
	message(STATUS "LIBDATA_${_PREFIX}_INCLUDE_DIR   = ${LIBDATA_${_PREFIX}_INCLUDE_DIR}")
	message(STATUS "LIBDATA_${_PREFIX}_LIBRARY       = ${LIBDATA_${_PREFIX}_LIBRARY}")

	# handle the QUIETLY and REQUIRED arguments and set component to TRUE
	# if all listed variables are TRUE
	find_package_handle_standard_args(${_MODNAME} DEFAULT_MSG
        LIBDATA_${_PREFIX}_INCLUDE_DIR
        LIBDATA_${_PREFIX}_LIBRARY
	)
	
    set(${_PREFIX}_PACKAGE_NAME     ${_PKGNAME})
	set(${_PREFIX}_INCLUDE_DIRS     ${LIBDATA_${_PREFIX}_INCLUDE_DIR})
	set(${_PREFIX}_LIBRARIES        ${LIBDATA_${_PREFIX}_LIBRARY})
	set(${_PREFIX}_DEFINITIONS      ${PCDATA_${_PREFIX}_CFLAGS_OTHER})

    message(STATUS "${_MODNAME}_FOUND           = ${${_MODNAME}_FOUND}")
    message(STATUS "${_PREFIX}_FOUND           = ${${_PREFIX}_FOUND}")
    message(STATUS "${_PREFIX}_PACKAGE_NAME    = ${${_PREFIX}_PACKAGE_NAME}")
    message(STATUS "${_PREFIX}_INCLUDE_DIRS    = ${${_PREFIX}_INCLUDE_DIRS}")
    message(STATUS "${_PREFIX}_LIBRARIES       = ${${_PREFIX}_LIBRARIES}")
    message(STATUS "${_PREFIX}_DEFINITIONS     = ${${_PREFIX}_DEFINITIONS}")
    
    unset(_PREFIX)
    
endmacro()
