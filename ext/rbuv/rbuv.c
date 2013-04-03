#include "rbuv.h"

VALUE mRbuv;

void Init_rbuv() {
  mRbuv = rb_define_module("Rbuv");
  Init_rbuv_error();
  Init_rbuv_handle();
  Init_rbuv_loop();
  Init_rbuv_timer();
}
