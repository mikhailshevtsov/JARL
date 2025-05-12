cmake_minimum_required(VERSION 3.28)
project(jarl LANGUAGES CXX)

add_library(jarl INTERFACE)
target_include_directories(jarl INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(jarl INTERFACE cxx_std_20)