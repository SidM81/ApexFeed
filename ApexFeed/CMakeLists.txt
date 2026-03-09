cmake_minimum_required(VERSION 3.16)
project(ApexFeed LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Threads REQUIRED)

add_library(apexfeed_core
    Parser/ITCH.cpp
    Socket/Socket.cpp
    OrderBook/OrderBook.cpp
)

target_include_directories(apexfeed_core PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(apexfeed_core PUBLIC Threads::Threads)

add_executable(apexfeed main.cpp)
target_link_libraries(apexfeed PRIVATE apexfeed_core)

add_executable(feed_ingest feed_ingest.cpp)
target_link_libraries(feed_ingest PRIVATE apexfeed_core Threads::Threads)

add_executable(test_feed_receiver Test/test_feed_receiver.cpp)
target_link_libraries(test_feed_receiver PRIVATE apexfeed_core)

add_executable(test_feed_sender Test/test_feed_sender.cpp)
target_link_libraries(test_feed_sender PRIVATE apexfeed_core)

add_executable(testmain Test/testmain.cpp)
target_link_libraries(testmain PRIVATE apexfeed_core)

add_executable(replay_feed Benchmarks/replay_feed.cpp)
target_link_libraries(replay_feed PRIVATE apexfeed_core)

add_executable(latency_bench Benchmarks/latency_bench.cpp)
target_link_libraries(latency_bench PRIVATE apexfeed_core)
