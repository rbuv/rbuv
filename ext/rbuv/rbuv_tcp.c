#include "rbuv_tcp.h"

struct rbuv_tcp_s {
  uv_tcp_t *uv_handle;
  VALUE cb_on_close;
  VALUE cb_on_connection;
  VALUE cb_on_read;
};

typedef struct {
  uv_connect_t *uv_connect;
  int status;
} _uv_tcp_on_connect_arg_t;

VALUE cRbuvTcp;

/* Allocator/deallocator */
static VALUE rbuv_tcp_s_new(int argc, VALUE *argv, VALUE klass);
static void rbuv_tcp_mark(rbuv_tcp_t *rbuv_tcp);
static void rbuv_tcp_free(rbuv_tcp_t *rbuv_tcp);

/* Methods */
/*
 uv_tcp_bind(uv_tcp_t* handle, struct sockaddr_in)
 uv_tcp_bind6(uv_tcp_t* handle, struct sockaddr_in6)
 uv_tcp_connect(uv_connect_t* req, uv_tcp_t* handle, struct sockaddr_in address, uv_connect❯
 uv_tcp_connect6(uv_connect_t* req, uv_tcp_t* handle, struct sockaddr_in6 address, uv_conne❯
 uv_tcp_getpeername(uv_tcp_t* handle, struct sockaddr* name, int* namelen)
 uv_tcp_getsockname(uv_tcp_t* handle, struct sockaddr* name, int* namelen)
 uv_tcp_init(uv_loop_t*, uv_tcp_t* handle)
 uv_tcp_keepalive(uv_tcp_t* handle, int enable, unsigned int delay)
 uv_tcp_nodelay(uv_tcp_t* handle, int enable)
 uv_tcp_open(uv_tcp_t* handle, uv_os_sock_t sock)
 uv_tcp_simultaneous_accepts(uv_tcp_t* handle, int enable)
 */
static VALUE rbuv_tcp_bind(VALUE self, VALUE ip, VALUE port);
//static VALUE rbuv_tcp_bind6(VALUE self, VALUE ip, VALUE port);
static VALUE rbuv_tcp_connect(VALUE self, VALUE ip, VALUE port);
//static VALUE rbuv_tcp_connect6(VALUE self, VALUE ip, VALUE port);

/* Private methods */
static void _uv_tcp_on_connect(uv_connect_t *uv_connect, int status);
static void _uv_tcp_on_connect_no_gvl(_uv_tcp_on_connect_arg_t *arg);
extern void __uv_stream_on_connection_no_gvl(uv_stream_t *uv_stream, int status);

void Init_rbuv_tcp() {
  cRbuvTcp = rb_define_class_under(mRbuv, "Tcp", cRbuvStream);
  rb_define_singleton_method(cRbuvTcp, "new", rbuv_tcp_s_new, -1);
  
  rb_define_method(cRbuvTcp, "bind", rbuv_tcp_bind, 2);
  //rb_define_method(cRbuvTcp, "bind6", rbuv_tcp_bind6, 2);
  rb_define_method(cRbuvTcp, "connect", rbuv_tcp_connect, 2);
  //rb_define_method(cRbuvTcp, "connect6", rbuv_tcp_connect6, 2);
}

VALUE rbuv_tcp_s_new(int argc, VALUE *argv, VALUE klass) {
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

  rbuv_tcp_t *rbuv_tcp = malloc(sizeof(*rbuv_tcp));
  rbuv_tcp->uv_handle = malloc(sizeof(*rbuv_tcp->uv_handle));
  rbuv_tcp->cb_on_close = Qnil;
  rbuv_tcp->cb_on_connection = Qnil;
  rbuv_tcp->cb_on_read = Qnil;
  
  VALUE tcp = Data_Wrap_Struct(klass, rbuv_tcp_mark, rbuv_tcp_free, rbuv_tcp);
  rbuv_tcp->uv_handle->data = (void *)tcp;

  uv_tcp_init(uv_loop, rbuv_tcp->uv_handle);

  rb_obj_call_init(tcp, argc, argv);
  
  RBUV_DEBUG_LOG_DETAIL("rbuv_tcp: %p, uv_handle: %p, tcp: %s",
                        rbuv_tcp, rbuv_tcp->uv_handle,
                        RSTRING_PTR(rb_inspect(tcp)));
  
  return tcp;
}

void rbuv_tcp_mark(rbuv_tcp_t *rbuv_tcp) {
  assert(rbuv_tcp);
  RBUV_DEBUG_LOG_DETAIL("rbuv_tcp: %p, uv_handle: %p, self: %lx",
                        rbuv_tcp, rbuv_tcp->uv_handle,
                        (VALUE)rbuv_tcp->uv_handle->data);
  rb_gc_mark(rbuv_tcp->cb_on_close);
  rb_gc_mark(rbuv_tcp->cb_on_connection);
  rb_gc_mark(rbuv_tcp->cb_on_read);
}

void rbuv_tcp_free(rbuv_tcp_t *rbuv_tcp) {
  RBUV_DEBUG_LOG_DETAIL("rbuv_tcp: %p, uv_handle: %p", rbuv_tcp, rbuv_tcp->uv_handle);

  if (!_rbuv_handle_is_closing((rbuv_handle_t *)rbuv_tcp)) {
    uv_close((uv_handle_t *)rbuv_tcp->uv_handle, NULL);
  }

  free(rbuv_tcp);
}

VALUE rbuv_tcp_bind(VALUE self, VALUE ip, VALUE port) {
  const char *uv_ip;
  int uv_port;
  rbuv_tcp_t *rbuv_tcp;
  struct sockaddr_in bind_addr;
  
  uv_ip = RSTRING_PTR(ip);
  uv_port = FIX2INT(port);
  
  bind_addr = uv_ip4_addr(uv_ip, uv_port);
  
  Data_Get_Struct(self, rbuv_tcp_t, rbuv_tcp);
  RBUV_CHECK_UV_RETURN(uv_tcp_bind(rbuv_tcp->uv_handle, bind_addr),
                       rbuv_tcp->uv_handle->loop);
  
  RBUV_DEBUG_LOG_DETAIL("self: %s, ip: %s, port: %d, rbuv_tcp: %p, uv_handle: %p",
                        RSTRING_PTR(rb_inspect(self)), uv_ip, uv_port, rbuv_tcp,
                        rbuv_tcp->uv_handle);
  
  return self;
}

VALUE rbuv_tcp_connect(VALUE self, VALUE ip, VALUE port) {
  VALUE block;
  const char *uv_ip;
  int uv_port;
  rbuv_tcp_t *rbuv_tcp;
  struct sockaddr_in connect_addr;
  uv_connect_t *uv_connect;

  rb_need_block();
  block = rb_block_proc();

  uv_ip = RSTRING_PTR(ip);
  uv_port = FIX2INT(port);

  Data_Get_Struct(self, rbuv_tcp_t, rbuv_tcp);
  rbuv_tcp->cb_on_connection = block;

  uv_connect = malloc(sizeof(*uv_connect));
  connect_addr = uv_ip4_addr(uv_ip, uv_port);

  RBUV_CHECK_UV_RETURN(uv_tcp_connect(uv_connect, rbuv_tcp->uv_handle,
                                      connect_addr, _uv_tcp_on_connect),
                       rbuv_tcp->uv_handle->loop);

  RBUV_DEBUG_LOG_DETAIL("self: %s, ip: %s, port: %d, rbuv_tcp: %p, uv_handle: %p",
                        RSTRING_PTR(rb_inspect(self)), uv_ip, uv_port, rbuv_tcp,
                        rbuv_tcp->uv_handle);

  return self;
}

void _uv_tcp_on_connect(uv_connect_t *uv_connect, int status) {
  _uv_tcp_on_connect_arg_t arg = { .uv_connect = uv_connect, .status = status };
  rb_thread_call_with_gvl((rbuv_rb_blocking_function_t)_uv_tcp_on_connect_no_gvl, &arg);
}

void _uv_tcp_on_connect_no_gvl(_uv_tcp_on_connect_arg_t *arg) {
  uv_connect_t *uv_connect = arg->uv_connect;
  int status = arg->status;

  __uv_stream_on_connection_no_gvl(uv_connect->handle, status);
  
  free(uv_connect);
}
