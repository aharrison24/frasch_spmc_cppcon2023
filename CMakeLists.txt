cmake_minimum_required(VERSION 3.22)

project(frasch_spsc_cppcon2023)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)

include(FetchContent)
FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG main)

FetchContent_Declare(googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG main # need main for benchmark::benchmark
)

set(BENCHMARK_ENABLE_TESTING OFF)

FetchContent_MakeAvailable(
        googletest
        googlebenchmark
)

add_compile_options(-Wall -Werror -Wextra -Wconversion)
add_compile_options(-Wno-unused-parameter)

function(add_fifo fifo)
    add_executable(${fifo} ${CMAKE_SOURCE_DIR}/${fifo}.cpp)

    add_executable(${fifo}.tsan ${CMAKE_SOURCE_DIR}/${fifo}.cpp)
    target_compile_options(${fifo}.tsan PRIVATE -fsanitize=thread)
    target_link_options(${fifo}.tsan PRIVATE -fsanitize=thread)
endfunction()


add_fifo(fifo1)
add_fifo(fifo2)
add_fifo(fifo3)
add_fifo(fifo4)
add_fifo(fifo4a)
add_fifo(fifo4b)
add_fifo(fifo5)
add_fifo(fifo5a)
add_fifo(boost_lockfree)
target_link_libraries(boost_lockfree PRIVATE Boost::boost)
target_link_libraries(boost_lockfree.tsan PRIVATE Boost::boost)

add_fifo(rigtorp)
add_fifo(mutex)
add_fifo(tryLock)


include(GoogleTest)
enable_testing()

add_executable(unitTests unitTests.cpp)
target_link_libraries(unitTests PRIVATE GTest::gtest_main)
target_compile_options(unitTests PRIVATE -fsanitize=undefined -fsanitize=address)
target_link_options(unitTests PRIVATE -fsanitize=undefined -fsanitize=address)
gtest_add_tests(TARGET unitTests)

add_executable(bench bench.cpp)
target_link_libraries(bench PRIVATE benchmark::benchmark)

add_executable(bench_all bench_all.cpp)
target_link_libraries(bench_all PRIVATE Boost::boost)
