# Copyright (C) 2018 Fondazione Istituto Italiano di Tecnologia
#
# Licensed under either the GNU Lesser General Public License v3.0 :
# https://www.gnu.org/licenses/lgpl-3.0.html
# or the GNU Lesser General Public License v2.1 :
# https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html
# at your option.

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

cmake_minimum_required(VERSION 3.11)

project(ALGLIB LANGUAGES CXX DESCRIPTION "A cross-platform numerical analysis and data processing library" HOMEPAGE_URL "https://www.alglib.net/")


include(AddWarningsConfigurationToTargets)
include(CMakePackageConfigHelpers)
include(AddInstallRPATHSupport)
include(FetchContent)

set(alglib_download_VERSION "4.08.0" CACHE STRING "ALGLIB download version (default: 4.08.0)")
set(alglib_SOURCE_DIR "${CMAKE_SOURCE_DIR}/alglib" CACHE PATH "Path to the ALGLIB sources (default: alglib)")
option(BUILD_SHARED_LIBS "Build libraries as shared as opposed to static" ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX "d")
endif()

set(CMAKE_INSTALL_BINDIR "bin")
set(CMAKE_INSTALL_LIBDIR "lib")
set(CMAKE_INSTALL_INCLUDEDIR "include")

option(ENABLE_RPATH "Enable RPATH for this library" ON)
mark_as_advanced(ENABLE_RPATH)
add_install_rpath_support(BIN_DIRS "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}" LIB_DIRS "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
                          DEPENDS ENABLE_RPATH USE_LINK_PATH)

if(NOT CMAKE_CONFIGURATION_TYPES)
  if(NOT CMAKE_BUILD_TYPE)
      message(STATUS "Setting build type to 'Release' as none was specified.")
      set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
  endif()
endif()

if(NOT EXISTS ${alglib_SOURCE_DIR})
    set(alglib_DOWNLOAD_LINK http://www.alglib.net/translator/re/alglib-${alglib_download_VERSION}.cpp.gpl.tgz)
    message(STATUS "Using ALGLIB sources from ${alglib_DOWNLOAD_LINK}")
    FetchContent_Declare(alglib URL ${alglib_DOWNLOAD_LINK} DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(alglib)
else()
    message(STATUS "Using ALGLIB sources from ${alglib_SOURCE_DIR}")
endif()
file(GLOB alglib_HEADERS "${alglib_SOURCE_DIR}/src/*.h")
file(GLOB alglib_SOURCES "${alglib_SOURCE_DIR}/src/*.cpp")

file(READ "${alglib_SOURCE_DIR}/src/ap.h" alglib_HEADER_CONTENT)
string(REGEX MATCH "ALGLIB[ \t]+([0-9]+\\.[0-9]+\\.[0-9]+)"
       _match
       "${alglib_HEADER_CONTENT}")
set(alglib_VERSION "${CMAKE_MATCH_1}")
message(STATUS "ALGLIB version: ${alglib_VERSION}")

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.cpp.in"
    "${CMAKE_BINARY_DIR}/version.cpp"
    @ONLY
)
list(APPEND alglib_SOURCES "${CMAKE_BINARY_DIR}/version.cpp")

add_library(alglib ${alglib_SOURCES})
target_include_directories(alglib PUBLIC "$<BUILD_INTERFACE:${alglib_SOURCE_DIR}/src>" "$<INSTALL_INTERFACE:include>")
set_property(TARGET alglib PROPERTY POSITION_INDEPENDENT_CODE ON)

set(COMPILER_OPTIONS)
if(MSVC OR MSYS OR MINGW)
    set(COMPILER_OPTIONS ${COMPILER_OPTIONS} /Ox /DAE_OS=AE_WINDOWS)
    execute_process( COMMAND wmic CPU get Name OUTPUT_VARIABLE CPU_NAME)
    if (${CPU_NAME} MATCHES "Intel")
        set(COMPILER_OPTIONS ${COMPILER_OPTIONS} /DAE_CPU=AE_INTEL)
    endif()
elseif(UNIX)
    set(COMPILER_OPTIONS ${COMPILER_OPTIONS} -O3 -DAE_OS=AE_POSIX -pthread)
endif()
target_compile_options(alglib PUBLIC ${COMPILER_OPTIONS})

# test
add_executable(minnlc_d_sparse src/minnlc_d_sparse.cpp)
target_link_libraries(minnlc_d_sparse PRIVATE alglib)

# install
set_target_properties(alglib PROPERTIES VERSION ${alglib_VERSION} PUBLIC_HEADER "${alglib_HEADERS}")
install(TARGETS alglib EXPORT alglib COMPONENT runtime PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT alglib DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/alglib)
configure_package_config_file(${CMAKE_SOURCE_DIR}/cmake/alglib-config.cmake.in ${CMAKE_BINARY_DIR}/alglib-config.cmake INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/alglib)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/alglib-configVersion.cmake VERSION ${alglib_VERSION} COMPATIBILITY ExactVersion)
install(FILES ${CMAKE_BINARY_DIR}/alglib-config.cmake ${CMAKE_BINARY_DIR}/alglib-configVersion.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/alglib)