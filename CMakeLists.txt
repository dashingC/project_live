cmake_minimum_required(VERSION 3.10)
project(tmms VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置安装目录
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/../bin)

# 编译选项，是Debug还是Release,Debug就-O0 -g（不优化），Release就-O3 -g（优化）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Werror -Wno-unused-variable")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -Wall -Werror -Wno-unused-variable")
endif()

# 添加通用编译选项
add_compile_options(-fno-strict-aliasing -Wno-error=shadow -Wno-sign-compare)

# GCC版本特定选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.2.9)
    add_compile_options(-Wno-error=unused-result -Werror=return-type)
endif()

find_package(Threads REQUIRED)

# 处理jsoncpp依赖
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(JSONCPP jsoncpp)
endif()

if(NOT JSONCPP_FOUND)
    # 使用ExternalProject作为备选
    include(ExternalProject)
    ExternalProject_Add(jsoncpp_external
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/jsoncpp
        CMAKE_ARGS 
            -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/jsoncpp
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_INSTALL_LIBDIR=lib
        BUILD_BYPRODUCTS ${CMAKE_BINARY_DIR}/jsoncpp/lib/libjsoncpp.a
    )
    
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/jsoncpp/include)

    add_library(jsoncpp STATIC IMPORTED)
    set_target_properties(jsoncpp PROPERTIES
        IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/jsoncpp/lib/libjsoncpp.a
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_BINARY_DIR}/jsoncpp/include
    )
    add_dependencies(jsoncpp jsoncpp_external)
endif()

# 包含源码目录
add_subdirectory(src)