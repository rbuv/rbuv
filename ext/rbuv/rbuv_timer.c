#include "rbuv_timer.h"

VALUE cRbuvTimer;

struct rbuv_timer_s {
  uv_timer_t *uv_handle;
  VALUE cb_on_close;
  VALUE cb_on_timeout;
};

typedef struct {
  uv_timer_t *uv_timer;
  int status;
} _uv_timer_on_timeout_no_gvl_arg_t;

/* Allocator/deallocator */
static VALUE rbuv_timer_s_new(int argc, VALUE *argv, VALUE klass);
static void rbuv_timer_mark(rbuv_timer_t *rbuv_timer);
static void rbuv_timer_free(rbuv_timer_t *rbuv_timer);

/* Methods */
static VALUE rbuv_timer_start(VALUE self, VALUE timeout, VALUE repeat);
static VALUE rbuv_timer_stop(VALUE self);
static VALUE rbuv_timer_repeat_get(VALUE self);
static VALUE rbuv_timer_repeat_set(VALUE self, VALUE repeat);

/* Private methods */
static void _uv_timer_on_timeout(uv_timer_t *uv_timer, int status);
static void _uv_timer_on_timeout_no_gvl(_uv_timer_on_timeout_no_gvl_arg_t *arg);

void Init_rbuv_timer() {
  cRbuvTimer = rb_define_class_under(mRbuv, "Timer", cRbuvHandle);
  rb_define_singleton_method(cRbuvTimer, "new", rbuv_timer_s_new, -1);

  rb_define_method(cRbuvTimer, "start", rbuv_timer_start, 2);
  rb_define_method(cRbuvTimer, "stop", rbuv_timer_stop, 0);
  rb_define_method(cRbuvTimer, "repeat", rbuv_timer_repeat_get, 0);
  rb_define_method(cRbuvTimer, "repeat=", rbuv_timer_repeat_set, 1);
}

VALUE rbuv_timer_s_new(int argc, VALUE *argv, VALUE klass) {
  VALUE loop;
  rb_scan_args(argc, argv, "01", &loop);

  uv_loop_t *uv_loop;
  if (loop == Qnil) {
    uv_loop = uv_default_loop();
  } else {
    rbuv_loop_t *rbuv_loop;
    Data_Get_Struct(loop, rbuv_loop_t, rbuv_loop);
    uv_loop = rbuv_loop->uv_handle;
  }

  rbuv_timer_t *rbuv_timer = malloc(sizeof(*rbuv_timer));
  rbuv_timer->uv_handle = malloc(sizeof(*rbuv_timer->uv_handle));
  rbuv_timer->cb_on_close = Qnil;
  rbuv_timer->cb_on_timeout = Qnil;

  VALUE timer = Data_Wrap_Struct(klass, rbuv_timer_mark, rbuv_timer_free, rbuv_timer);
  rbuv_timer->uv_handle->data = (void *)timer;

  uv_timer_init(uv_loop, rbuv_timer->uv_handle);

  rb_obj_call_init(timer, argc, argv);

  return timer;
}

void rbuv_timer_mark(rbuv_timer_t *rbuv_timer) {
  assert(rbuv_timer);
  rb_gc_mark(rbuv_timer->cb_on_close);
  rb_gc_mark(rbuv_timer->cb_on_timeout);
}

void rbuv_timer_free(rbuv_timer_t *rbuv_timer) {
  assert(rbuv_timer);
  RBUV_DEBUG_LOG_DETAIL("rbuv_timer: %p, uv_handle: %p", rbuv_timer, rbuv_timer->uv_handle);
  if (!_rbuv_handle_is_closing((rbuv_handle_t *)rbuv_timer)) {
    uv_close((uv_handle_t *)rbuv_timer->uv_handle, NULL);
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
  rbuv_timer->cb_on_timeout = block;
  
  RBUV_DEBUG_LOG_DETAIL("rbuv_timer: %p, uv_handle: %p, _uv_timer_on_timeout: %p, timer: %s",
                        rbuv_timer, rbuv_timer->uv_handle, _uv_timer_on_timeout,
                        RSTRING_PTR(rb_inspect(self)));
  uv_timer_start(rbuv_timer->uv_handle, _uv_timer_on_timeout,
                 uv_timeout, uv_repeat);

  return self;
}

/**
 * stop the timer.
 * @return self
 */
VALUE rbuv_timer_stop(VALUE self) {
  rbuv_timer_t *rbuv_timer;

  Data_Get_Struct(self, rbuv_timer_t, rbuv_timer);
  
  uv_timer_stop(rbuv_timer->uv_handle);
  
  return self;
}

VALUE rbuv_timer_repeat_get(VALUE self) {
  rbuv_timer_t *rbuv_timer;
  VALUE repeat;
  
  Data_Get_Struct(self, rbuv_timer_t, rbuv_timer);
  repeat = ULL2NUM(uv_timer_get_repeat(rbuv_timer->uv_handle));
  
  return repeat;
}

VALUE rbuv_timer_repeat_set(VALUE self, VALUE repeat) {
  rbuv_timer_t *rbuv_timer;
  uint64_t uv_repeat;
  
  uv_repeat = NUM2ULL(repeat);
  
  Data_Get_Struct(self, rbuv_timer_t, rbuv_timer);
  
  uv_timer_set_repeat(rbuv_timer->uv_handle, uv_repeat);
  
  return repeat;
}

void _uv_timer_on_timeout(uv_timer_t *uv_timer, int status) {
  _uv_timer_on_timeout_no_gvl_arg_t reg = { .uv_timer = uv_timer, .status = status };
  rb_thread_call_with_gvl((rbuv_rb_blocking_function_t)_uv_timer_on_timeout_no_gvl, &reg);
}

void _uv_timer_on_timeout_no_gvl(_uv_timer_on_timeout_no_gvl_arg_t *arg) {
  uv_timer_t *uv_timer = arg->uv_timer;
  int status = arg->status;

  VALUE timer;
  rbuv_timer_t *rbuv_timer;
  
  timer = (VALUE)uv_timer->data;
  Data_Get_Struct(timer, struct rbuv_timer_s, rbuv_timer);
  
  rb_funcall(rbuv_timer->cb_on_timeout, id_call, 1, timer);
}
