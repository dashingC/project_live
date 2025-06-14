add_executable(tmms main.cpp)

target_link_libraries(tmms 
    PRIVATE 
        base
        network
)

# 安装规则
install(TARGETS tmms 
    RUNTIME DESTINATION sbin
)