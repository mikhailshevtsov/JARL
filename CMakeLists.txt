cmake_minimum_required(VERSION 3.28)
project(jarl LANGUAGES CXX)

add_library(jarl INTERFACE)
target_include_directories(jarl INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(jarl INTERFACE cxx_std_20)

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.8.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(jarl_tests test/main.cpp)
target_link_libraries(jarl_tests PRIVATE Catch2::Catch2WithMain)