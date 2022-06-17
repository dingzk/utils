cmake_minimum_required(VERSION 3.15)
project(utils)

include_directories(../)
AUX_SOURCE_DIRECTORY(../ UTILS_SOURCE)

add_executable(test_thread_pool test_thread_pool.cpp ${UTILS_SOURCE})
target_link_libraries(test_thread_pool libboost_system.a libboost_thread.a pthread)



