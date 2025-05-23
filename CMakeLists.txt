cmake_minimum_required(VERSION 3.22)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")

include_directories(src/include)

add_executable(code
        src/storage/b_plus_tree_leaf_page.cpp
        src/storage/b_plus_tree_page.cpp
        src/storage/b_plus_tree_internal_page.cpp
        src/storage/page_guard.cpp
        src/storage/b_plus_tree.cpp
        src/disk/disk_manager.cpp
        src/buffer/lru_k_replacer.cpp
        src/buffer/buffer_pool_manager.cpp
        src/include/common/map.h
        src/include/common/vector.h
        src/include/common/list.h
        src/management/user.cpp
        src/include/management/user.h
        src/include/common/util.h
        src/management/main.cpp
        src/management/train.cpp
        src/include/management/train.h
        src/include/management/management.h
        src/management/management.cpp
        src/include/management/ticket.h
        src/management/ticket.cpp
)