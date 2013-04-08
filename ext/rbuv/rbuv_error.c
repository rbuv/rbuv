#include "rbuv_error.h"

VALUE eRbuvError;

void Init_rbuv_error() {
  eRbuvError = rb_define_class_under(mRbuv, "Error", rb_eStandardError);
}
