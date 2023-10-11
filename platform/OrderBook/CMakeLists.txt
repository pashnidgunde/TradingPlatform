project(OrderBook LANGUAGES CXX)

add_library(${PROJECT_NAME}
        include/OrderedLinkedList.h
        include/OrderBookFields.h
        include/OrderBook.h
        include/Order.h
        include/OutgoingEvents.h
        include/NodePool.h
        include/OrderBookHandler.h
        include/IncomingEvents.h
        OrderBookHandler.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ./include)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Platform")

add_subdirectory(test)