# googletest的使用文档：https://google.github.io/googletest

if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
    set(DOWNLOAD_EXTRACT_TIMESTAMP ON)
endif ()

include(FetchContent)
FetchContent_Declare(
        googletest
        URL ${CMAKE_CURRENT_LIST_DIR}/googletest-1.14.0.tar.gz
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
