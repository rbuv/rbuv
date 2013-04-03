#ifndef RBUV_H_
#define RBUV_H_

#include <assert.h>

#include <ruby.h>
#include <uv.h>

#include "debug.h"

#include "error.h"
#include "handle.h"
#include "loop.h"
#include "timer.h"

extern ID id_call;

extern VALUE mRbuv;

#define RBUV_CHECK_UV_RETURN(uv_ret) do { \
  if (uv_ret) { \
    uv_err_t err = uv_last_error(uv_default_loop()); \
    rb_raise(eRbuvError, "%s", uv_strerror(err)); \
  } \
} while(0)

#endif  /* RBUV_H_ */