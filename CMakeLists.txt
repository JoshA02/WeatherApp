cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(WeatherApp)

find_package(nlohmann_json REQUIRED)
find_package(curl REQUIRED)
find_package(curlcpp REQUIRED)
find_package(Boost REQUIRED)

# Set the source directory
set(SRC_DIR "src")
set(TEST_DIR "tests")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

# Find all .cpp files in the source and test directories
file(GLOB SOURCES "${SRC_DIR}/*.cpp")
file(GLOB TESTS "${TEST_DIR}/*.cpp")

# Find all .cpp files in the source directory except main.cpp
file(GLOB NON_MAIN_SOURCES "${SRC_DIR}/*.cpp")
list(REMOVE_ITEM NON_MAIN_SOURCES "${SRC_DIR}/main.cpp")

add_executable(WeatherApp ${SOURCES})
target_link_libraries(WeatherApp PRIVATE nlohmann_json curlcpp::curlcpp curl)

add_executable(WeatherAppTest ${TESTS} ${NON_MAIN_SOURCES})
target_include_directories(WeatherAppTest PRIVATE ${SRC_DIR})
target_link_libraries(WeatherAppTest PRIVATE nlohmann_json curlcpp::curlcpp curl Boost::boost)
target_compile_definitions(WeatherAppTest PRIVATE TEST_MODE)
target_compile_options(WeatherAppTest PRIVATE -fprofile-arcs -ftest-coverage)