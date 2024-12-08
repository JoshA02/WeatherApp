# Weather App

## Requirements
- vcpkg
- C++ 20
- lcov, gcov, genhtml

## Running
1) Create the makefile via `cmake --preset=default`.
2) Build the project with `cmake --build build`.
3) Execute the regular program via `./build/WeatherApp`, or the unit tests with `./build/WeatherAppTest --log_level=message`.

## Checking Coverage
1) Upon compiling and executing the test application (`./build/WeatherAppTest --log_level=message`), run `cmake --build build --target coverage`.
2) This will create a few files within the `./temp_coverage_output` directory.
3) To review code coverage in a simple-to-read format, open `./temp_coverage_output/results/index.html` in any web browser.

## Note
If you receive any errors relating to `gcda` and/or `gcno` files, try running `cmake --build build --target clean_coverage` and retrying.