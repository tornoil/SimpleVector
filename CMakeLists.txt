cmake_minimum_required(VERSION 3.0.0)
project(simple_vector_2 VERSION 0.1.0)

add_executable(seventh_sprint main.cpp)
target_compile_options(seventh_sprint
                        PRIVATE
                        -std=c++17
                        -Werror
                        -Wall)

