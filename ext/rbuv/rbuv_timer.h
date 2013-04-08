#ifndef RBUV_TIMER_H_
#define RBUV_TIMER_H_

#include "rbuv.h"
#include "rbuv_handle.h"

typedef struct rbuv_timer_s rbuv_timer_t;

extern VALUE cRbuvTimer;

void Init_rbuv_timer();

#endif  /* RBUV_TIMER_H_ */
