# 1. 指定 CMake 最低版本
cmake_minimum_required(VERSION 3.10)

# 2. 定义项目名称
project(MyFileSystem C)

# 3. 设置 C 语言标准 (C99)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 4. 设置路径变量 
# 告诉 CMake 头文件在 include 目录
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
# 告诉 CMake 源文件在 src 目录
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 5. 定义源文件列表
set(SOURCES
    ${SRC_DIR}/main.c
    ${SRC_DIR}/my_format.c
    ${SRC_DIR}/fs_utils.c
    ${SRC_DIR}/fs_persistence.c
    ${SRC_DIR}/my_shell.c
    ${SRC_DIR}/my_ls.c
    ${SRC_DIR}/my_cd.c
    ${SRC_DIR}/my_close.c
    ${SRC_DIR}/my_open.c
    ${SRC_DIR}/my_create.c
    ${SRC_DIR}/my_mkdir.c
    ${SRC_DIR}/my_rmdir.c
    ${SRC_DIR}/my_write.c
    ${SRC_DIR}/my_read.c
    ${SRC_DIR}/my_rm.c
)

# 6. 生成可执行文件
add_executable(myfs ${SOURCES})

# 7. 指定头文件包含目录
target_include_directories(myfs PRIVATE ${INC_DIR})