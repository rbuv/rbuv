#include "handle.h"

struct rbuv_handle_s {
  uv_handle_t *uv_handle;
  VALUE cb_on_close;
};

VALUE cRbuvHandle;

/* Methods */
static VALUE rbuv_handle_close(VALUE self);
static VALUE rbuv_handle_is_active(VALUE self);
static VALUE rbuv_handle_is_closing(VALUE self);

/* Private methods */
static void _uv_handle_close(uv_handle_t *uv_handle);
static void _uv_handle_on_close(uv_handle_t *uv_handle);

void Init_rbuv_handle() {
  cRbuvHandle = rb_define_class_under(mRbuv, "Handle", rb_cObject);
  rb_undef_alloc_func(cRbuvHandle);
  
  rb_define_method(cRbuvHandle, "close", rbuv_handle_close, 0);
  rb_define_method(cRbuvHandle, "active?", rbuv_handle_is_active, 0);
  rb_define_method(cRbuvHandle, "closing?", rbuv_handle_is_closing, 0);
}

VALUE rbuv_handle_close(VALUE self) {
  rbuv_handle_t *rbuv_handle;
  VALUE block;

  if (rb_block_given_p()) {
    block = rb_block_proc();
  } else {
    block = Qnil;
  }

  Data_Get_Struct(self, rbuv_handle_t, rbuv_handle);

  _rbuv_handle_close(rbuv_handle);

  return Qnil;
}

VALUE rbuv_handle_is_active(VALUE self) {
  rbuv_handle_t *rbuv_handle;
  
  Data_Get_Struct(self, rbuv_handle_t, rbuv_handle);
  
  return _rbuv_handle_is_active(rbuv_handle) ? Qtrue : Qfalse;
}

VALUE rbuv_handle_is_closing(VALUE self) {
  rbuv_handle_t *rbuv_handle;

  Data_Get_Struct(self, rbuv_handle_t, rbuv_handle);

  return _rbuv_handle_is_closing(rbuv_handle) ? Qtrue : Qfalse;
}

int _rbuv_handle_is_active(rbuv_handle_t *rbuv_handle) {
  assert(rbuv_handle);
  return uv_is_active(rbuv_handle->uv_handle);
}

int _rbuv_handle_is_closing(rbuv_handle_t *rbuv_handle) {
  assert(rbuv_handle);
  return uv_is_closing(rbuv_handle->uv_handle);
}

void _rbuv_handle_close(rbuv_handle_t *rbuv_handle) {
  assert(rbuv_handle);
  RBUV_DEBUG_LOG_DETAIL("rbuv_handle: %p, uv_handle: %p",
                        rbuv_handle, rbuv_handle->uv_handle);
  if (!_rbuv_handle_is_closing(rbuv_handle)) {
    RBUV_DEBUG_LOG_DETAIL("closing rbuv_handle: %p, uv_handle: %p",
                          rbuv_handle, rbuv_handle->uv_handle);
    _uv_handle_close(rbuv_handle->uv_handle);
  }
}

void _uv_handle_close(uv_handle_t *uv_handle) {
  assert(uv_handle);
  uv_close(uv_handle, _uv_handle_on_close);
}

void _uv_handle_on_close(uv_handle_t *uv_handle) {
  VALUE handle;
  rbuv_handle_t *rbuv_handle;
  VALUE on_close;

  handle = (VALUE)uv_handle->data;

  RBUV_DEBUG_LOG_DETAIL("uv_handle: %p, handle: %s", uv_handle, RSTRING_PTR(rb_inspect(handle)));

  Data_Get_Struct(handle, rbuv_handle_t, rbuv_handle);

  on_close = rbuv_handle->cb_on_close;

  RBUV_DEBUG_LOG_DETAIL("handle: %s, on_close: %s", RSTRING_PTR(rb_inspect(handle)),
                        RSTRING_PTR(rb_inspect(on_close)));

  if (RTEST(on_close)) {
    rb_funcall(on_close, id_call, 1, handle);
  }
}
