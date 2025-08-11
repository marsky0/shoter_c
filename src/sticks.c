#include "sticks.h"

void sticks_generator_new_event(SticksGenerator* sg, StickArray* sa, Trade* t, u64 realloc_size) {
	u64 index = t->timestamp / sg->interval;

	if (sg->temp_stick.time == 0) {
		sg->temp_stick.time = index;
		sg->temp_stick.high = t->price;
		sg->temp_stick.avg = t->price;
		sg->temp_stick.low = t->price;
		if (!t->is_market) {
			sg->temp_stick.volume_buy = t->quantity*t->price;
			sg->temp_stick.volume_sell = 0.0;
			sg->temp_stick.trades_buy = 1;
			sg->temp_stick.trades_sell = 0;
		} else {
			sg->temp_stick.volume_buy = 0.0;			
			sg->temp_stick.volume_sell =  t->quantity*t->price;
			sg->temp_stick.trades_buy = 0;
			sg->temp_stick.trades_sell = 1;
		}
	} else {
		if (sg->temp_stick.time < index) {
            u64 capacity = index - sg->temp_stick.time;
            if (sa->capacity - sa->length <= capacity+1) {
                sa->sticks = realloc(sa->sticks, sizeof(Stick)*(sa->capacity + capacity+realloc_size) );
                sa->capacity += capacity+realloc_size;
            }
			
            u64 base_time = (sg->temp_stick.time + 1) * sg->interval;
			for (i32 i=0; i < capacity-1; i++) {
				Stick *stick = &sa->sticks[sa->length + i];
                stick->time = base_time + i * sg->interval;
                stick->high = stick->avg = stick->low = t->price;
                stick->volume_buy = stick->volume_sell = 0.0;
                stick->trades_buy = stick->trades_sell = 0;
			}
            sa->length += capacity-1;

			Stick *last = &sa->sticks[sa->length];
            last->time = (sg->temp_stick.time + 1) * sg->interval;
            last->high = sg->temp_stick.high;
            last->avg = sg->temp_stick.avg / (sg->temp_stick.trades_buy + sg->temp_stick.trades_sell);
            last->low = sg->temp_stick.low;
            last->volume_buy = sg->temp_stick.volume_buy;
            last->volume_sell = sg->temp_stick.volume_sell;
            last->trades_buy = sg->temp_stick.trades_buy;
            last->trades_sell = sg->temp_stick.trades_sell;
            sa->length++;


			sg->temp_stick.time = index;
			sg->temp_stick.high = t->price;
			sg->temp_stick.avg = t->price;
			sg->temp_stick.low = t->price;
			if (!t->is_market) {
    			sg->temp_stick.volume_buy = t->quantity * t->price;
    			sg->temp_stick.trades_buy = 1;
    			sg->temp_stick.volume_sell = 0.0;
    			sg->temp_stick.trades_sell = 0;
			} else {
    			sg->temp_stick.volume_buy = 0.0;
    			sg->temp_stick.trades_buy = 0;
    			sg->temp_stick.volume_sell = t->quantity * t->price;
    			sg->temp_stick.trades_sell = 1;
			}
		} else {
			sg->temp_stick.high = (t->price > sg->temp_stick.high) ? t->price : sg->temp_stick.high;
			sg->temp_stick.avg += t->price;
			sg->temp_stick.low = (t->price < sg->temp_stick.low) ? t->price : sg->temp_stick.low;
			if (!t->is_market) {
				sg->temp_stick.volume_buy += t->quantity*t->price;
				sg->temp_stick.trades_buy += 1;
			} else {
				sg->temp_stick.volume_sell +=  t->quantity*t->price;
				sg->temp_stick.trades_sell += 1;
			}
		}
	}
}

