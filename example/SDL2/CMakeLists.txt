CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

set(CMAKE_CXX_STANDARD 14)

PROJECT(lunaSVG_examples LANGUAGES CXX)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../include/" )

ADD_EXECUTABLE(lunaSVG_SDL2 lunaSVG_SDL2.cpp)
target_link_libraries(lunaSVG_SDL2 ${SDL2_LIBRARIES} lunasvg)


