#include "market.h"
#include "lib/typedefs.h"

#ifndef STICKS_H
#define STICKS_H

typedef struct Stick {
	f64 high;
	f64 avg;
	f64 low;
	f64 volume_buy;
	f64 volume_sell;
	u64 time;
	u64 trades_buy;
	u64 trades_sell;
} Stick;

typedef struct StickArray {
	Stick* sticks;
	u64 capacity;
	u64 length;
} StickArray;

typedef struct StickData {
    string symbol;
    f64 start_price;
    f64 peak_price;
    f64 post_price;
} StickData;

typedef struct StickDataArra {
    StickData* data;
	u64 capacity;
	u64 length;
} StickDataArray;

typedef struct SticksGenerator {
	u64 interval;
	Stick temp_stick;
} SticksGenerator;

void sticks_generator_new_event(SticksGenerator* sg, StickArray* sa, Trade* t, u64 realloc_size);

#endif
