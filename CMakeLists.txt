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

# Custom target for generating code coverage
set(COVERAGE_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/temp_coverage_output)
file(MAKE_DIRECTORY ${COVERAGE_OUTPUT_DIR})
file(MAKE_DIRECTORY ${COVERAGE_OUTPUT_DIR}/results)
add_custom_target(
    coverage

    # rm all previously outputted coverage files using find and rm (remove every non-directory file)
    COMMAND ${CMAKE_COMMAND} -E echo "Removing old coverage files if any..."
    COMMAND find ${COVERAGE_OUTPUT_DIR} -type f -exec rm -f {} + || true
    


    COMMAND ${CMAKE_COMMAND} -E echo "Generating code coverage..."
    COMMAND ${CMAKE_COMMAND} -E echo ${CMAKE_BINARY_DIR}
    COMMAND lcov -c -d ${CMAKE_BINARY_DIR}/CMakeFiles/WeatherAppTest.dir -o ${COVERAGE_OUTPUT_DIR}/coverage.info --ignore-errors inconsistent --rc derive_function_end_line=0 > ${COVERAGE_OUTPUT_DIR}/lcov_output.txt
    COMMAND lcov --remove ${COVERAGE_OUTPUT_DIR}/coverage.info --output-file ${COVERAGE_OUTPUT_DIR}/filtered_coverage.info '*/vcpkg_installed/*' '/usr/lib/*' '/usr/include/*' '*.h' --rc derive_function_end_line=0 || true
    COMMAND genhtml -o ${COVERAGE_OUTPUT_DIR}/results ${COVERAGE_OUTPUT_DIR}/filtered_coverage.info --rc derive_function_end_line=0 --ignore-errors category >> ${COVERAGE_OUTPUT_DIR}/lcov_output.txt
    COMMAND ${CMAKE_COMMAND} -E echo "Code coverage generated. Open temp_coverage_output/results/index.html in a browser to view the results."
)

add_custom_target(
    clean_coverage

    COMMAND ${CMAKE_COMMAND} -E echo "Removing any existing .gcda and .gcno files..."
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -exec rm -f {} +
    COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcno" -exec rm -f {} +
)