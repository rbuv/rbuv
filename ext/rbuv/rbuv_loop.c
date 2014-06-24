#include "rbuv_loop.h"

typedef struct {
  uv_loop_t* loop;
  int mode;
} _rbuv_loop_run_arg_t;

VALUE cRbuvLoop;
static ID intern_ivarDefault;

/* Methods */
static VALUE rbuv_loop_run(VALUE loop);
static VALUE rbuv_loop_stop(VALUE loop);
static VALUE rbuv_loop_run_once(VALUE loop);
static VALUE rbuv_loop_run_nowait(VALUE loop);
static VALUE rbuv_loop_s_default(VALUE klass);

/* Allocator/deallocator */
static VALUE rbuv_loop_alloc(VALUE klass);
static void rbuv_loop_mark(rbuv_loop_t *rbuv_loop);
static void rbuv_loop_free(rbuv_loop_t *rbuv_loop);

/* Private methods */
static void _rbuv_loop_run(VALUE self, uv_run_mode mode);
static void _rbuv_loop_run_no_gvl(_rbuv_loop_run_arg_t *arg);

void Init_rbuv_loop() {
  cRbuvLoop = rb_define_class_under(mRbuv, "Loop", cRbuvHandle);
  rb_define_alloc_func(cRbuvLoop, rbuv_loop_alloc);

  rb_define_method(cRbuvLoop, "run", rbuv_loop_run, 0);
  rb_define_method(cRbuvLoop, "stop", rbuv_loop_stop, 0);
  rb_define_method(cRbuvLoop, "run_once", rbuv_loop_run_once, 0);
  rb_define_method(cRbuvLoop, "run_nowait", rbuv_loop_run_nowait, 0);
  rb_define_singleton_method(cRbuvLoop, "default", rbuv_loop_s_default, 0);

  intern_ivarDefault = rb_intern("@default");
}

VALUE rbuv_loop_alloc(VALUE klass) {
  rbuv_loop_t *rbuv_loop;
  VALUE loop;

  rbuv_loop = malloc(sizeof(*rbuv_loop));
  rbuv_loop->uv_handle = uv_loop_new();
  rbuv_loop->is_default = false;

  loop = Data_Wrap_Struct(klass, rbuv_loop_mark, rbuv_loop_free, rbuv_loop);
  rbuv_loop->uv_handle->data = (void *)loop;

  RBUV_DEBUG_LOG_DETAIL("rbuv_loop: %p, uv_handle: %p, loop: %s",
                        rbuv_loop, rbuv_loop->uv_handle,
                        RSTRING_PTR(rb_inspect(loop)));

  return loop;
}

void rbuv_loop_mark(rbuv_loop_t *rbuv_loop) {
  assert(rbuv_loop);
  RBUV_DEBUG_LOG_DETAIL("rbuv_loop: %p, uv_handle: %p, self: %lx",
                        rbuv_loop, rbuv_loop->uv_handle,
                        (VALUE)rbuv_loop->uv_handle->data);
}

void rbuv_loop_free(rbuv_loop_t *rbuv_loop) {
  RBUV_DEBUG_LOG_DETAIL("rbuv_loop: %p, uv_handle: %p", rbuv_loop, rbuv_loop->uv_handle);

  if (!rbuv_loop->is_default) {
    uv_loop_delete(rbuv_loop->uv_handle);
  }

  free(rbuv_loop);
}

VALUE rbuv_loop_s_default(VALUE klass) {
  VALUE loop = rb_ivar_defined(klass, intern_ivarDefault) ? rb_ivar_get(klass, intern_ivarDefault) : Qnil;
  if (loop == Qnil) {
    rbuv_loop_t *rbuv_loop;

    rbuv_loop = malloc(sizeof(*rbuv_loop));
    rbuv_loop->uv_handle = uv_default_loop();
    rbuv_loop->is_default = true;

    loop = Data_Wrap_Struct(klass, rbuv_loop_mark, rbuv_loop_free, rbuv_loop);
    rbuv_loop->uv_handle->data = (void *)loop;

    RBUV_DEBUG_LOG_DETAIL("rbuv_loop: %p, uv_handle: %p, loop: %s",
                          rbuv_loop, rbuv_loop->uv_handle,
                          RSTRING_PTR(rb_inspect(loop)));

    rb_ivar_set(klass, intern_ivarDefault, loop);
  }
  return loop;
}

VALUE rbuv_loop_run(VALUE self) {
  _rbuv_loop_run(self, UV_RUN_DEFAULT);
  return Qnil;
}

VALUE rbuv_loop_stop(VALUE self) {
  rbuv_loop_t *rbuv_loop;

  Data_Get_Struct(self, rbuv_loop_t, rbuv_loop);

  uv_stop(rbuv_loop->uv_handle);
  return Qnil;
}

VALUE rbuv_loop_run_once(VALUE self) {
  _rbuv_loop_run(self, UV_RUN_ONCE);
  return Qnil;
}

VALUE rbuv_loop_run_nowait(VALUE self) {
  _rbuv_loop_run(self, UV_RUN_NOWAIT);
  return Qnil;
}

void _rbuv_loop_run(VALUE self, uv_run_mode mode) {
  rbuv_loop_t *rbuv_loop;

  Data_Get_Struct(self, rbuv_loop_t, rbuv_loop);
  _rbuv_loop_run_arg_t arg = { .mode = mode, .loop = rbuv_loop->uv_handle};
#ifdef HAVE_RB_THREAD_CALL_WITHOUT_GVL
  rb_thread_call_without_gvl((rbuv_rb_blocking_function_t)_rbuv_loop_run_no_gvl,
                             &arg, RUBY_UBF_IO, 0);
#else
  rb_thread_blocking_region((rb_blocking_function_t *)_rbuv_loop_run_no_gvl,
                            &arg, RUBY_UBF_IO, 0);
#endif
}

void _rbuv_loop_run_no_gvl(_rbuv_loop_run_arg_t *arg) {
  uv_run(arg->loop, arg->mode);
}
