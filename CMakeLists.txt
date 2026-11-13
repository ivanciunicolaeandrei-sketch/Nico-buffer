cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(NicoBuffer VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK.")
endif()

# Geode SDK 5.4.1 accidentally declares CCFileUtilsWin32::init() twice.
# Remove only the duplicate declaration before the SDK headers are compiled.
set(GEODE_FILEUTILS_HEADER "$ENV{GEODE_SDK}/loader/include/Geode/cocos/platform/win32/CCFileUtilsWin32.h")
if (EXISTS "${GEODE_FILEUTILS_HEADER}")
    file(READ "${GEODE_FILEUTILS_HEADER}" GEODE_FILEUTILS_CONTENT)
    string(FIND "${GEODE_FILEUTILS_CONTENT}" "    virtual bool init();\n\n    /**" GEODE_DUPLICATE_INIT_POSITION)
    if (NOT GEODE_DUPLICATE_INIT_POSITION EQUAL -1)
        string(REPLACE "    virtual bool init();\n\n    /**" "    /**" GEODE_FILEUTILS_CONTENT "${GEODE_FILEUTILS_CONTENT}")
        file(WRITE "${GEODE_FILEUTILS_HEADER}" "${GEODE_FILEUTILS_CONTENT}")
        message(STATUS "Patched duplicate CCFileUtilsWin32::init declaration")
    endif()
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)
setup_geode_mod(${PROJECT_NAME})
