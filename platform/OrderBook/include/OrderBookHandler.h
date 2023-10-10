#pragma once
#include "OrderBook.h"
struct OrderBookHandler {
  void onEvent();
  void pre();
  void handle();
  void post();
};
