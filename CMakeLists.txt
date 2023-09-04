cmake_minimum_required (VERSION 3.24)
project (TradingPlatform LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -Wextra -Wpedantic")
#set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Werror -Wall -Wextra -Wpedantic")

add_subdirectory(platform)
#add_subdirectory(services)

enable_testing()
find_package(GTest CONFIG REQUIRED)

# for arranging projects under seperate folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_target_properties(OrderCache PROPERTIES FOLDER "Platform")
set_target_properties(TestOrderCache PROPERTIES FOLDER "Tests")