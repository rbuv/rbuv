#include "handle.h"

struct rbuv_handle_s {
  uv_handle_t *uv_handle;
};

VALUE cRbuvHandle;

/* Private methods */
static void _uv_handle_on_close(uv_handle_t *uv_handle);

void Init_rbuv_handle() {
  cRbuvHandle = rb_define_class_under(mRbuv, "Handle", rb_cObject);
  rb_undef_alloc_func(cRbuvHandle);
}

void rbuv_handle_close(rbuv_handle_t *rbuv_handle) {
  assert(rbuv_handle);
  assert(rbuv_handle->uv_handle);
  uv_close(rbuv_handle->uv_handle, _uv_handle_on_close);
}

void _uv_handle_on_close(uv_handle_t *uv_handle) {
  free(uv_handle);
}