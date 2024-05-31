# googletest的使用文档：https://github.com/google/glog?tab=readme-ov-file#getting-started

if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
    set(DOWNLOAD_EXTRACT_TIMESTAMP ON)
endif ()

include(FetchContent)
FetchContent_Declare(
        glog
        URL ${CMAKE_CURRENT_LIST_DIR}/glog-0.7.0.tar.gz
)

# 关闭gtest编译
set(WITH_GFLAGS OFF)
set(BUILD_SHARED_LIBS OFF)
add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)
FetchContent_MakeAvailable(glog)
