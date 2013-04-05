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
#include "stream.h"
#include "tcp.h"

extern ID id_call;

extern VALUE mRbuv;

#define RBUV_CHECK_UV_RETURN(uv_ret) do { \
  if (uv_ret) { \
    uv_err_t err = uv_last_error(uv_default_loop()); \
    rb_raise(eRbuvError, "%s", uv_strerror(err)); \
  } \
} while(0)

#define RBUV_OFFSETOF(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define RBUV_CONTAINTER_OF(ptr, type, member) ({ \
  const typeof( ((type *)0)->member ) *__mptr = (ptr); \
  (type *)( (char *)__mptr - RBUV_OFFSETOF(type, member) );})

#endif  /* RBUV_H_ */