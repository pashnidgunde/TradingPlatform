project(OrderBook LANGUAGES CXX)

add_library(${PROJECT_NAME}
        include/OrderedLinkedList.h
        include/OrderBookFields.h
        include/OrderBookLL.h
        include/Order.h
        include/OutgoingEvents.h
        include/NodePool.h
        include/IncomingEvents.h
        OrderBook.cpp
        include/OrderBook.h
        include/LinkedList.h
)

target_include_directories(${PROJECT_NAME} PRIVATE ./include)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Platform")

add_subdirectory(test)