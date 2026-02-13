cmake_minimum_required(VERSION 4.0)

project(Median_Calculator CXX)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

set(BOOST_ENABLE_CMAKE ON)
set(BOOST_INCLUDE_LIBRARIES program_options accumulators)
FetchContent_Declare(
    boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-1.84.0
)

set(SPDLOG_BUILD_SHARED OFF)
set(SPDLOG_BUILD_EXAMPLE OFF)
set(SPDLOG_BUILD_TESTS OFF)
set(SPDLOG_BUILD_BENCH OFF)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
# add_compile_definitions(TOML_ENABLE_ORDERED_MAP=1)

FetchContent_MakeAvailable(
    boost
    spdlog
    tomlplusplus
)

add_executable(Median_Calculator
    Sources/main.cpp
    Sources/csv_reader.cpp
    Sources/config_parser.cpp
    Sources/median_calculator.cpp
)

target_link_libraries(Median_Calculator PRIVATE
    Boost::program_options
    Boost::accumulators
    tomlplusplus::tomlplusplus
    spdlog::spdlog_header_only)
# target_link_libraries(main PRIVATE fmt::fmt)