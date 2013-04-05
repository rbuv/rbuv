#include "handle.h"

struct rbuv_handle_s {
  uv_handle_t *uv_handle;
};

VALUE cRbuvHandle;

/* Methods */
static VALUE rbuv_handle_is_active(VALUE self);

/* Private methods */
static void _uv_handle_close(uv_handle_t *uv_handle);
static void _uv_handle_on_close(uv_handle_t *uv_handle);

void Init_rbuv_handle() {
  cRbuvHandle = rb_define_class_under(mRbuv, "Handle", rb_cObject);
  rb_undef_alloc_func(cRbuvHandle);
  
  rb_define_method(cRbuvHandle, "active?", rbuv_handle_is_active, 0);
}

VALUE rbuv_handle_is_active(VALUE self) {
  rbuv_handle_t *rbuv_handle;
  
  Data_Get_Struct(self, rbuv_handle_t, rbuv_handle);
  
  return uv_is_active(rbuv_handle->uv_handle) ? Qtrue : Qfalse;
}

void rbuv_handle_close(rbuv_handle_t *rbuv_handle) {
  assert(rbuv_handle);
  _uv_handle_close(rbuv_handle->uv_handle);
}

void _uv_handle_close(uv_handle_t *uv_handle) {
  assert(uv_handle);
  uv_close(uv_handle, _uv_handle_on_close);
}

void _uv_handle_on_close(uv_handle_t *uv_handle) {
  RBUV_DEBUG_LOG_DETAIL("uv_handle: %p, handle: %ld",
                        uv_handle, (VALUE)uv_handle->data);
  free(uv_handle);
}