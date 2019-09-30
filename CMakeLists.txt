
cmake_minimum_required (VERSION 2.6)
project (ltevTool)


add_definitions(-std=c++11)

# 生成目标名
set(TARGET ltevTool)
# 添加头文件目录 ，相当于Makefile的 -I
include_directories(
    ${CMAKE_SOURCE_DIR}/asn1/src
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/cjson
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/message
    ${CMAKE_SOURCE_DIR}/src/message/map
    ${CMAKE_SOURCE_DIR}/src/message/rsi
)

# 库路径 ， 相当于Makefile的 -L
link_directories(
    ${CMAKE_SOURCE_DIR}/asn1
)

add_executable(${TARGET}
    src/main.c
    src/common/common.c
    src/cjson/cJSON.c
    src/message/msg_common.c
    src/message/map/map_check_jsion.c
    src/message/map/map_encode.c
    src/message/map/map_print.c
    src/message/rsi/rsi_about.c
)

target_link_libraries(${TARGET}
    ltev_asn1
    m
)




