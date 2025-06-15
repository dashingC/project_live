# 收集 network 目录及其子目录下的所有源文件
file(GLOB_RECURSE NETWORK_SOURCES "*.cpp" "*.h")

# 创建一个名为 network 的静态库
add_library(network STATIC ${NETWORK_SOURCES})

# 设置头文件包含目录
# PUBLIC 关键字确保了任何链接到 network 库的目标，也会自动包含这些头文件路径
target_include_directories(network 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}  # 包含 network/ 目录
        ${CMAKE_SOURCE_DIR}/src      # 包含 src/ 目录，以便访问 base 等模块
)

# 链接 network 库所依赖的其他库
# 这里的 base 是您在 src/base/CMakeLists.txt 中定义的库
# Threads::Threads 是通过 find_package(Threads) 找到的线程库
target_link_libraries(network 
    PUBLIC 
        base
        Threads::Threads
)

add_executable(EventLoopThreadTest
    net/tests/EventLoopThreadTest.cpp
)

target_link_libraries(EventLoopThreadTest
    PRIVATE
        network 
)

add_executable(InetAddressTest
    net/tests/InetAddressTest.cpp
)

target_link_libraries(InetAddressTest
    PRIVATE
        network 
)

add_executable(SockOptTest
    net/tests/SockOptTest.cpp
)

target_link_libraries(SockOptTest
    PRIVATE
        network 
)
