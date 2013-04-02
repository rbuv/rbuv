#include "loop.h"

struct rbuv_handle_s {
  uv_handle_t *uv_handle;
};

VALUE cRbuvLoop;

/* Methods */
static VALUE rbuv_loop_s_run(VALUE klass);
static VALUE rbuv_loop_s_stop(VALUE klass);
static VALUE rbuv_loop_s_run_once(VALUE klass);
static VALUE rbuv_loop_s_run_nowait(VALUE klass);

/* Private methods */
static void _rbuv_loop_run(uv_run_mode mode);

void Init_rbuv_loop() {
  cRbuvLoop = rb_define_class_under(mRbuv, "Loop", cRbuvHandle);
  rb_undef_alloc_func(cRbuvLoop);

  rb_define_singleton_method(cRbuvLoop, "run", rbuv_loop_s_run, 0);
  rb_define_singleton_method(cRbuvLoop, "stop", rbuv_loop_s_stop, 0);
  rb_define_singleton_method(cRbuvLoop, "run_once", rbuv_loop_s_run_once, 0);
  rb_define_singleton_method(cRbuvLoop, "run_nowait", rbuv_loop_s_run_nowait, 0);
}

VALUE rbuv_loop_s_run(VALUE klass) {
  _rbuv_loop_run(UV_RUN_DEFAULT);
  return Qnil;
}

VALUE rbuv_loop_s_stop(VALUE klass) {
  uv_stop(uv_default_loop());
  return Qnil;
}

VALUE rbuv_loop_s_run_once(VALUE klass) {
  _rbuv_loop_run(UV_RUN_ONCE);
  return Qnil;
}

VALUE rbuv_loop_s_run_nowait(VALUE klass) {
  _rbuv_loop_run(UV_RUN_NOWAIT);
  return Qnil;
}

void _rbuv_loop_run(uv_run_mode mode) {
  uv_run(uv_default_loop(), mode);
}
