
cmake_minimum_required (VERSION 2.6)
project (ltevTool)

# c++11
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

# 生成目标名
set(TARGET ltevTool)

# 添加头文件目录 ，相当于Makefile的 -I
include_directories(
    asn/src
    src
    src/map
    src/rsi
)

# 库路径 ， 相当于Makefile的 -L
link_directories(
    ${CMAKE_SOURCE_DIR}/asn
)

add_executable(${TARGET}
    main.cpp
    src/common.cpp
    src/cJSON.c
    src/convert_common.cpp
    src/map/map_check_jsion.cpp
    src/map/map_encode.cpp
    src/map/map_print.cpp
    src/rsi/rsi_about.cpp
)

target_link_libraries(${TARGET}
    ltev_asn_2020
    m
)

#add_subdirectory(asn)



