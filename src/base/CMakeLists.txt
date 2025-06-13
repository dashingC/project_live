aux_source_directory(. DIR_LIB_SRCS)      #将当前目录下的所有源文件添加到DIR_LIB_SRCS变量中
add_library (base ${DIR_LIB_SRCS})        #创建一个名为base的库，包含DIR_LIB_SRCS中的源文件
add_executable(base_test ${DIR_LIB_SRCS}) #创建一个名为base_test的可执行文件，包含DIR_LIB_SRCS中的源文件