#include "NewOrderBook.h"

//std::ostream &operator<<(std::ostream &os, const std::optional<TopOfBook<SIDE_BUY>> &TopOfBooksRAII) {
//    if (!TopOfBooksRAII.has_value()) {
//        os << TopOfBook<SIDE_BUY>::value << ", " << TopOfBook<SIDE_BUY>::side << ", -" << ", -";
//    } else {
//        os << TopOfBooksRAII.value();
//    }
//    return os;
//}
//
//std::ostream &operator<<(std::ostream &os, const std::optional<TopOfBook<SIDE_SELL>> &TopOfBooksRAII) {
//    if (!TopOfBooksRAII.has_value()) {
//        os << TopOfBook<SIDE_SELL>::value << ", " << TopOfBook<SIDE_SELL>::side << ", -" << ", -";
//    } else {
//        os << TopOfBooksRAII.value();
//    }
//    return os;
//}