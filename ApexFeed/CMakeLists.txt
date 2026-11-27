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

set(APEXALPHA_DIR ${CMAKE_SOURCE_DIR}/../ApexAlpha)

add_library(apexfeed_core
    Parser/ITCH.cpp
    Socket/Socket.cpp
    OrderBook/OrderBook.cpp
)

target_include_directories(apexfeed_core PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(apexfeed_core PUBLIC Threads::Threads)

add_library(apexalpha_core
    ${APEXALPHA_DIR}/Feature/FeatureEngine.cpp
    ${APEXALPHA_DIR}/Strategy/StrategyConfig.cpp
    ${APEXALPHA_DIR}/Strategy/ThresholdStrategy.cpp
    ${APEXALPHA_DIR}/Simulation/ExecutionSimulator.cpp
    ${APEXALPHA_DIR}/Simulation/PnlTracker.cpp
    ${APEXALPHA_DIR}/Pipeline/SymbolRouter.cpp
    ${APEXALPHA_DIR}/Pipeline/AlphaEngine.cpp
    ${APEXALPHA_DIR}/IO/CsvWriter.cpp
    ${APEXALPHA_DIR}/IO/RunSummary.cpp
)

target_include_directories(apexalpha_core PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/..)
target_link_libraries(apexalpha_core PUBLIC apexfeed_core)

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

add_executable(replay_strategy ${APEXALPHA_DIR}/Apps/replay_strategy.cpp)
target_link_libraries(replay_strategy PRIVATE apexalpha_core)

add_executable(latency_bench Benchmarks/latency_bench.cpp)
target_link_libraries(latency_bench PRIVATE apexfeed_core)

add_executable(test_apexalpha_features ${APEXALPHA_DIR}/Tests/test_apexalpha_features.cpp)
target_link_libraries(test_apexalpha_features PRIVATE apexalpha_core)

add_executable(test_apexalpha_router ${APEXALPHA_DIR}/Tests/test_apexalpha_router.cpp)
target_link_libraries(test_apexalpha_router PRIVATE apexalpha_core)

add_executable(test_apexalpha_strategy_sim ${APEXALPHA_DIR}/Tests/test_apexalpha_strategy_sim.cpp)
target_link_libraries(test_apexalpha_strategy_sim PRIVATE apexalpha_core)
