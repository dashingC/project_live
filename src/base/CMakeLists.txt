# 收集源文件
file(GLOB_RECURSE BASE_SOURCES "*.cpp" "*.h")

# 创建base库
add_library(base STATIC ${BASE_SOURCES})

# 设置包含目录
target_include_directories(base 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖
target_link_libraries(base 
    PUBLIC 
        jsoncpp
        Threads::Threads
        rt
)

# 如果需要测试可执行文件
option(BUILD_BASE_TESTS "Build base tests" OFF)
if(BUILD_BASE_TESTS)
    add_executable(base_test ${BASE_SOURCES})
    target_link_libraries(base_test base)
endif()


