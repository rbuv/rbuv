#include "timer.h"

static ID id_call;

VALUE cRbuvTimer;

struct rbuv_timer_s {
  uv_timer_t *uv_handle;
  VALUE cb;
};

/* Allocator/deallocator */
static VALUE rbuv_timer_alloc(VALUE klass);
static void rbuv_timer_mark(rbuv_timer_t *rbuv_timer);
static void rbuv_timer_free(rbuv_timer_t *rbuv_timer);

/* Methods */
static VALUE rbuv_timer_start(VALUE self, VALUE timeout, VALUE repeat);
static VALUE rbuv_timer_stop(VALUE self);
static VALUE rbuv_timer_is_active(VALUE self);

/* Private methods */
static void _uv_timer_on_timeout(uv_timer_t *uv_timer, int status);
static int _rbuv_timer_is_active(rbuv_timer_t *rbuv_timer);

void Init_rbuv_timer() {
  id_call = rb_intern("call");

  cRbuvTimer = rb_define_class_under(mRbuv, "Timer", cRbuvHandle);
  rb_define_alloc_func(cRbuvTimer, rbuv_timer_alloc);

  rb_define_method(cRbuvTimer, "start", rbuv_timer_start, 2);
  rb_define_method(cRbuvTimer, "stop", rbuv_timer_stop, 0);
  rb_define_method(cRbuvTimer, "active?", rbuv_timer_is_active, 0);
}

VALUE rbuv_timer_alloc(VALUE klass) {
  rbuv_timer_t *rbuv_timer;
  VALUE timer;

  rbuv_timer = malloc(sizeof(*rbuv_timer));
  rbuv_timer->uv_handle = NULL;

  timer = Data_Wrap_Struct(klass, rbuv_timer_mark, rbuv_timer_free, rbuv_timer);

  return timer;
}

void rbuv_timer_mark(rbuv_timer_t *rbuv_timer) {
  assert(rbuv_timer);
  rb_gc_mark(rbuv_timer->cb);
}

void rbuv_timer_free(rbuv_timer_t *rbuv_timer) {
  assert(rbuv_timer);
  if (rbuv_timer->uv_handle) {
    rbuv_handle_close((rbuv_handle_t *)rbuv_timer);
  }
  free(rbuv_timer);
}

/**
 * start the timer.
 * @param timeout the timeout in millisecond.
 * @param repeat the repeat interval in millisecond.
 * @return self
 */
VALUE rbuv_timer_start(VALUE self, VALUE timeout, VALUE repeat) {
  VALUE block;
  uint64_t uv_timeout;
  uint64_t uv_repeat;
  rbuv_timer_t *rbuv_timer;
  
  rb_need_block();
  block = rb_block_proc();
  uv_timeout = NUM2ULL(timeout);
  uv_repeat = NUM2ULL(repeat);
  
  Data_Get_Struct(self, rbuv_timer_t, rbuv_timer);
  rbuv_timer->cb = block;
  
  rbuv_timer->uv_handle = malloc(sizeof(*rbuv_timer->uv_handle));
  uv_timer_init(uv_default_loop(), rbuv_timer->uv_handle);
  rbuv_timer->uv_handle->data = (void *)self;
  
  uv_timer_start(rbuv_timer->uv_handle, _uv_timer_on_timeout,
                 uv_timeout, uv_repeat);

#ifndef RBUV_RBX
  rb_gc_mark(self);
#endif
  return self;
}

VALUE rbuv_timer_stop(VALUE self) {
  rbuv_timer_t *rbuv_timer;

  Data_Get_Struct(self, rbuv_timer_t, rbuv_timer);
  
  uv_timer_stop(rbuv_timer->uv_handle);
  
  return self;
}

VALUE rbuv_timer_is_active(VALUE self) {
  rbuv_timer_t *rbuv_timer;

  Data_Get_Struct(self, rbuv_timer_t, rbuv_timer);
  
  return _rbuv_timer_is_active(rbuv_timer) ? Qtrue : Qfalse;
}

void _uv_timer_on_timeout(uv_timer_t *uv_timer, int status) {
  VALUE timer;
  rbuv_timer_t *rbuv_timer;
  
  timer = (VALUE)uv_timer->data;
  Data_Get_Struct(timer, struct rbuv_timer_s, rbuv_timer);
  
  rb_funcall(rbuv_timer->cb, id_call, 1, timer);
}

int _rbuv_timer_is_active(struct rbuv_timer_s *rbuv_timer) {
  assert(rbuv_timer);
  return uv_is_active((uv_handle_t *)rbuv_timer->uv_handle);
}