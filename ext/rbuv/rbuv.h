#ifndef RBUV_H_
#define RBUV_H_

#include <assert.h>
#include <stdlib.h>

#include <ruby.h>
#ifdef HAVE_RUBY_THREAD_H
# include <ruby/thread.h>
#endif
#include <uv.h>

#include "rbuv_debug.h"

#include "rbuv_error.h"
#include "rbuv_handle.h"
#include "rbuv_loop.h"
#include "rbuv_timer.h"
#include "rbuv_stream.h"
#include "rbuv_tcp.h"
#include "rbuv_signal.h"

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

typedef void *(*rbuv_rb_blocking_function_t)(void *);

#endif  /* RBUV_H_ */
