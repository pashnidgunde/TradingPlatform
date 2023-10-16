project(OrderMatcher LANGUAGES CXX)

add_library(${PROJECT_NAME}
        include/SymbolResolver.h
        include/MessageQueue.h
        OrderMatcher.cpp
        include/MessageHandler.h
)

target_include_directories(${PROJECT_NAME} PUBLIC ../OrderBook/include)
target_include_directories(${PROJECT_NAME} PRIVATE ./include)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Platform")