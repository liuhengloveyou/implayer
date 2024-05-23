if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
    set(DOWNLOAD_EXTRACT_TIMESTAMP ON)
endif ()

include(FetchContent)
FetchContent_Declare(
        SDL2
        URL ${CMAKE_CURRENT_LIST_DIR}/SDL-release-2.30.3.zip
)

set(BUILD_SHARED_LIBS OFF)
set(protobuf_BUILD_TESTS OFF)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    SET(protobuf_MSVC_STATIC_RUNTIME OFF)
endif ()
FetchContent_MakeAvailable(SDL2)