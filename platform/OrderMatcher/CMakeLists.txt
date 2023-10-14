project(OrderMatcher LANGUAGES CXX)

add_library(${PROJECT_NAME}
        include/SymbolResolver.h
        include/IncomingEvents.h
        include/OrderMatcher.h
        OrderMatcher.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC ../OrderBook/include)
target_include_directories(${PROJECT_NAME} PRIVATE ./include)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Platform")