project(MatchingEngine LANGUAGES CXX)

find_package(Boost REQUIRED)

add_executable(${PROJECT_NAME}
        MatchingEngine.cpp
)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ../../platform/OrderMatcher/include)
link_directories(${PROJECT_NAME} {Boost_LIBRARY_DIRS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "service")
