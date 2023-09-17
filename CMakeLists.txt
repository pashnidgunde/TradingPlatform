cmake_minimum_required (VERSION 3.22)
project (TradingPlatform LANGUAGES CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -Wextra -Wpedantic")
#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -Wextra -Wpedantic")

add_subdirectory(platform)
#add_subdirectory(services)

# for arranging projects under seperate folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


