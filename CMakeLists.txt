cmake_minimum_required(VERSION 3.14)
project(Processor)

set(CMAKE_CXX_STANDARD 17)

add_executable(Processor main.cpp Translator/Translator.h CPU/Processor.h CPU/MemoryController.h CPU/IOController.h)