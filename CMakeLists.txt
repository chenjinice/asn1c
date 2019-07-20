
cmake_minimum_required (VERSION 2.6)
project (ltev_tool)


# 生成目标名
set(TARGET ltev_tool)

# 添加头文件目录 ，相当于Makefile的 -I
include_directories(
    ${CMAKE_SOURCE_DIR}/asn1/src
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/cjson
)

# 库路径 ， 相当于Makefile的 -L
link_directories(
    ${CMAKE_SOURCE_DIR}/asn1
)

add_executable(${TARGET}
    src/main.c
    src/encode.c
    src/decode.c
	cjson/cJSON.c
)

target_link_libraries(${TARGET}
	ltev_asn1
        m
)




