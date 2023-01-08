################################################################################
#                                                                              #
# Copyright (c) 2011-2014, University of Delaware                              # 
# All rights reserved.                                                         #
#                                                                              #
# Redistribution and use in source and binary forms, with or without           # 
# modification, are permitted provided that the following conditions           # 
# are met:                                                                     #
#                                                                              #
# 1. Redistributions of source code must retain the above copyright            # 
# notice, this list of conditions and the following disclaimer.                # 
#                                                                              #
# 2. Redistributions in binary form must reproduce the above copyright         # 
# notice, this list of conditions and the following disclaimer in the          # 
# documentation and/or other materials provided with the distribution.         # 
#                                                                              #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS          # 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT            # 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS            # 
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE               # 
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,         # 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,         # 
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;             # 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER             # 
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT           # 
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN            # 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE              # 
# POSSIBILITY OF SUCH DAMAGE.                                                  # 
#                                                                              #
################################################################################

cmake_minimum_required( VERSION 2.6 )

ENABLE_TESTING()

project ( darts )

if( "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local" )
message( "HOME" ) 
set (CMAKE_INSTALL_PREFIX $ENV{HOME} )
endif( "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local" )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Look for supported threading libraries
find_package( Threads REQUIRED )

if( UNIX )
    if( NOT CMAKE_USE_PTHREADS_INIT )
        message( FATAL_ERROR "Require pthreads on Unix-like systems" )
    endif( NOT CMAKE_USE_PTHREADS_INIT )
elseif( WIN32 )
    if( NOT CMAKE_USE_WIN32_THREADS_INIT )
        message( FATAL_ERROR "Require win32 threads on Windows systems" )
    endif( NOT CMAKE_USE_WIN32_THREADS_INIT )
else( WIN32 )
    message( FATAL_ERROR "Unsupported operating system" )
endif( UNIX )

find_package( Hwloc 1.11 REQUIRED )
option (NEW_HWLOC "NEW_HWLOC" ON)
if( HWLOC_FOUND )
    include_directories( ${HWLOC_INCLUDE_DIR} )
    link_directories( ${HWLOC_LIBRARY_DIRS} )
    message (" -- " ${HWLOC_INCLUDE_DIR})
    if (NEW_HWLOC)
        add_definitions( -DNEW_HWLOC )
    endif()
endif( HWLOC_FOUND )

#  INCLUDE_DIRECTORIES(/Users/celsloaner/repos/darts/sw/hwloc/include)
#  LINK_DIRECTORIES(/Users/celsloaner/repos/darts/sw/hwloc/lib)

option(TBB "TBB" ON)
if(TBB)
find_package( TBB )
if ( TBB_FOUND )
    add_definitions( -DTBB )
    include_directories( ${TBB_INCLUDE_DIR} )
    link_directories( ${TBB_LIBRARY_DIRS} )
endif ( TBB_FOUND )
else()
    message("DEQUE ON")
endif(TBB)

option(MKL "MKL" OFF)
option(ACML "ACML" OFF)
option(CBLAS "CBLAS" ON)
option(MKL_ROOT_DIR "Root directory of the MKL library" "/opt/intel/mkl/")
if(MKL)
    message("mkl ON")
    message("mkl location = " ${MKL_ROOT_DIR})
    add_definitions( -DMKL )

    include_directories(${MKL_ROOT_DIR}/include)
    link_directories(${MKL_ROOT_DIR}/lib/intel64)
elseif(ACML)
    message("ACML ON")
else(CBLAS)
    add_definitions( -DCBLAS )
    message("CBLAS ON")
    include_directories(/usr/local/Cellar/openblas/0.3.7/include/)
    link_directories(/usr/local/Cellar/openblas/0.3.7/lib/)
endif(MKL)

include_directories(include/abstractmachine
                    include/common
                    include/runtime
                    include/scheduler 
                    include/threading
                    include/threadlocal
                    include/threadsafe )

option(DEBUG "DEBUG" OFF)
if(DEBUG)
message("DEBUG VERSION!!!")
set (CMAKE_BUILD_TYPE "DEBUG")
else()
message("Release Version")
set (CMAKE_BUILD_TYPE "RELEASE")
endif()

set (CMAKE_CXX_FLAGS_DEBUG "-g -pg")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

## Compiler flags
if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
    set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -std=c++11  -Wall -fopenmp -Wextra -Wno-pmf-conversions" )
elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" )
    set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -Wall -Wextra -std=c++11 " )
    set( CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} "-fsanitize=address")
elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC" )
    set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "/W3 /wd4995" )
else()
    message( FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}" )
endif()

option(TRACE "TRACE" OFF)
if(TRACE)
message("Tracing ON!")
add_definitions(-DTRACE)
else()
message("Tracing off")
endif(TRACE)

option(TRACE "COUNT" OFF)
if(COUNT)
message("Counting ON!")
add_definitions(-DCOUNT)
include_directories( /home/software/papi/5.1.1/include )
link_directories( /home/software/papi/5.1.1/lib )
else()
message("Counting off")
endif(COUNT)

add_subdirectory( include )
add_subdirectory( src )
add_subdirectory( apps )

