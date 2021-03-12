CMAKE_MINIMUM_REQUIRED(VERSION 3.7)


PROJECT(lunaSVG_examples LANGUAGES CXX)

find_package(wxWidgets COMPONENTS core base REQUIRED)
include( "${wxWidgets_USE_FILE}" )

include_directories(${wxWidgets_INCLUDE_DIRS} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../../include/" )

ADD_EXECUTABLE(lunaSVG_WX lunaSVG_WX.cpp)
target_link_libraries(lunaSVG_WX ${wxWidgets_LIBRARIES} lunasvg)



