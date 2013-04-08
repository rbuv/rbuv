#include "rbuv.h"

ID id_call;

VALUE mRbuv;

void Init_rbuv() {
  id_call = rb_intern("call");

  mRbuv = rb_define_module("Rbuv");
  Init_rbuv_error();
  Init_rbuv_handle();
  Init_rbuv_loop();
  Init_rbuv_timer();
  Init_rbuv_stream();
  Init_rbuv_tcp();
  Init_rbuv_signal();
}
