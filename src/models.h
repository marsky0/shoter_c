#include <stdbool.h>
#include "lib/typedefs.h"

#ifndef MODELS_H
#define MODELS_H

typedef struct {
	u64 id;
	u64 timestamp;
	f64 price;
	f64 quantity;
	bool is_market;
} Trade;

typedef struct {
	Trade* trades;
	u64 capacity;
	u64 length;
} TradesArray;

typedef enum {
	New,
    PartiallyFilled,
    Filled,
    Canceled,
    Rejected,
    Expired,
} OrderStatus;

typedef enum {
	Buy,
	Sell,
} OrderSide;

typedef enum {
	Limit,
	Market,
	// TakeProfit #TODO
	// Stop, #TODO
	StopMarket,
	TakeProfitMarket
} OrderType;

typedef enum {
	Long,
	Short
} PositionSide;

typedef enum {
	GTC
} TimeInForce;

typedef struct {
	char* client_order_id;
	u64 order_id;
	char* symbol;
	OrderSide side;
	PositionSide position_side;
	OrderType order_type;
	OrderStatus status;
	f64 pirce;
	f64 avg_price;
	f64 orig_qty;
	f64 executed_qty;
	f64 stop_price;
	TimeInForce time_in_force;
	u64 update_time;
	u64 time;
} Order;

typedef struct {} Position; // #TODO

#endif
