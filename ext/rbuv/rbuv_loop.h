#ifndef RBUV_LOOP_H_
#define RBUV_LOOP_H_

#include "rbuv.h"

struct rbuv_loop_s {
  uv_loop_t* uv_handle;
  bool is_default;
};
typedef struct rbuv_loop_s rbuv_loop_t;

extern VALUE cRbuvLoop;

void Init_rbuv_loop();

#endif  /* RBUV_LOOP_H_ */
