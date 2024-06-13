cmake_minimum_required(VERSION 3.28)
project(compareNO_OCR)

set(CMAKE_CXX_STANDARD 17)

set(POPPLER_DIR /opt/homebrew/Cellar/poppler/24.04.0) # Set up the library path
include_directories(${POPPLER_DIR}/include/) # include the lib
link_directories(${POPPLER_DIR}/lib/)


add_executable(compareNO_OCR main.cpp
        Question.cpp
        Question.h
        form.cpp
        form.h)

target_link_libraries(compareNO_OCR poppler-cpp)
