cmake_minimum_required(VERSION 3.15)
project(Grammar)

set(CMAKE_CXX_STANDARD 14)

add_executable(Grammar main.cpp headers/DataStruct.h Symbol.cpp headers/Symbol.h headers/Status.h LALR1.cpp headers/LALR1.h headers/Mapping.h Mapping.cpp headers/PreDefined.h Translation.cpp headers/Translation.h)