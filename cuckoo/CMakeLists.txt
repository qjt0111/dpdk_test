
cmake_minimum_required(VERSION 3.0)
project(main)


set(CMAKE_C_FLAGS "-std=c99")


set(SRC_LIST cuckoo.c hash.c)
set(cuckoo_include /home/qjt/dpdk_test/cuckoo)

set(CXX_FLAGS
    -g
)


include_directories($(cuckoo_include))


add_executable(demo ${SRC_LIST})
