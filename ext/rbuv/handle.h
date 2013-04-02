#ifndef RBUV_HANDLE_H_
#define RBUV_HANDLE_H_

#include "rbuv.h"

typedef struct rbuv_handle_s rbuv_handle_t;

extern VALUE cRbuvHandle;

void Init_rbuv_handle();

void rbuv_handle_close(rbuv_handle_t *rbuv_handle);

#endif  /* RBUV_HANDLE_H_ */