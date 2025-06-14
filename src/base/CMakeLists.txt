# 收集源文件
file(GLOB_RECURSE BASE_SOURCES "*.cpp" "*.h")

# 创建一个名为base静态库库
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

add_executable(base_test ${BASE_SOURCES})
target_link_libraries(base_test base)