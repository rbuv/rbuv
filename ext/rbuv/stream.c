#include "stream.h"

struct rbuv_stream_s {
  uv_stream_t *uv_handle;
  VALUE cb_on_connection;
  VALUE cb_on_read;
};

typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} rbuv_write_req_t;

VALUE cRbuvStream;

/* Methods */
static VALUE rbuv_stream_listen(VALUE self, VALUE backlog);
static VALUE rbuv_stream_accept(VALUE self, VALUE client);
static VALUE rbuv_stream_is_readable(VALUE self);
static VALUE rbuv_stream_is_writable(VALUE self);
static VALUE rbuv_stream_shutdown(VALUE self);
static VALUE rbuv_stream_read_start(VALUE self);
//static VALUE rbuv_stream_read2_start(VALUE self, VALUE client);
static VALUE rbuv_stream_read_stop(VALUE self);
static VALUE rbuv_stream_write(VALUE self, VALUE data);
//static VALUE rbuv_stream_write2(VALUE self);

/* Private methods */
static void _uv_stream_on_connection(uv_stream_t *server, int status);
static uv_buf_t _uv_alloc_buffer(uv_handle_t *handle, size_t suggested_size);
static void _uv_stream_on_read(uv_stream_t *stream, ssize_t nread, uv_buf_t buf);
static void _uv_stream_on_write(uv_write_t *req, int status);

void Init_rbuv_stream() {
  cRbuvStream = rb_define_class_under(mRbuv, "Stream", cRbuvHandle);
  rb_undef_alloc_func(cRbuvStream);
  
  rb_define_method(cRbuvStream, "listen", rbuv_stream_listen, 1);
  rb_define_method(cRbuvStream, "accept", rbuv_stream_accept, 1);
  rb_define_method(cRbuvStream, "readable?", rbuv_stream_is_readable, 0);
  rb_define_method(cRbuvStream, "writable?", rbuv_stream_is_writable, 0);
  rb_define_method(cRbuvStream, "shutdown", rbuv_stream_shutdown, 0);
  rb_define_method(cRbuvStream, "read_start", rbuv_stream_read_start, 0);
//  rb_define_method(cRbuvStream, "read2_start", rbuv_stream_read2_start, 0);
  rb_define_method(cRbuvStream, "read_stop", rbuv_stream_read_stop, 0);
  rb_define_method(cRbuvStream, "write", rbuv_stream_write, 1);
//  rb_define_method(cRbuvStream, "write2", rbuv_stream_write2, 1);
}

VALUE rbuv_stream_listen(VALUE self, VALUE backlog) {
  rbuv_stream_t *rbuv_server;
  VALUE block;
  int uv_backlog;
  
  rb_need_block();
  block = rb_block_proc();
  
  Data_Get_Struct(self, rbuv_stream_t, rbuv_server);
  rbuv_server->cb_on_connection = block;
  
  uv_backlog = FIX2INT(backlog);
  
  RBUV_DEBUG_LOG_DETAIL("self: %ld, backlog: %d, block: %ld, rbuv_server: %p, uv_handle: %p, _uv_stream_on_connection: %p",
                        self, uv_backlog, block, rbuv_server, rbuv_server->uv_handle, _uv_stream_on_connection);
  uv_listen(rbuv_server->uv_handle, uv_backlog, _uv_stream_on_connection);
  
  return self;
}

VALUE rbuv_stream_accept(VALUE self, VALUE client) {
  rbuv_stream_t *rbuv_server;
  rbuv_stream_t *rbuv_client;
  
  Data_Get_Struct(self, rbuv_stream_t, rbuv_server);
  Data_Get_Struct(client, rbuv_stream_t, rbuv_client);
  
  RBUV_CHECK_UV_RETURN(uv_accept(rbuv_server->uv_handle, rbuv_client->uv_handle));
  
  return self;
}

VALUE rbuv_stream_is_readable(VALUE self) {
  rbuv_stream_t *rbuv_stream;
  
  Data_Get_Struct(self, rbuv_stream_t, rbuv_stream);
  
  return uv_is_readable(rbuv_stream->uv_handle) ? Qtrue : Qfalse;
}

VALUE rbuv_stream_is_writable(VALUE self) {
  rbuv_stream_t *rbuv_stream;
  
  Data_Get_Struct(self, rbuv_stream_t, rbuv_stream);
  
  return uv_is_writable(rbuv_stream->uv_handle) ? Qtrue : Qfalse;
}

VALUE rbuv_stream_shutdown(VALUE self) {
  rbuv_stream_t *rbuv_stream;
  
  Data_Get_Struct(self, rbuv_stream_t, rbuv_stream);
  
  rb_raise(rb_eNotImpError, __func__);
  
  return self;
}

VALUE rbuv_stream_read_start(VALUE self) {
  rbuv_stream_t *rbuv_stream;
  VALUE block;
  
  rb_need_block();
  block = rb_block_proc();
  
  Data_Get_Struct(self, rbuv_stream_t, rbuv_stream);
  rbuv_stream->cb_on_read = block;
  
  uv_read_start(rbuv_stream->uv_handle, _uv_alloc_buffer, _uv_stream_on_read);
  
  return self;
}

VALUE rbuv_stream_read_stop(VALUE self) {
  rbuv_stream_t *rbuv_stream;

  Data_Get_Struct(self, rbuv_stream_t, rbuv_stream);

  uv_read_stop(rbuv_stream->uv_handle);

  return self;
}

VALUE rbuv_stream_write(VALUE self, VALUE data) {
  rbuv_stream_t *rbuv_stream;
  rbuv_write_req_t *req;
  
  Data_Get_Struct(self, rbuv_stream_t, rbuv_stream);
  
  req = malloc(sizeof(*req));
  
  req->buf = uv_buf_init((char *)malloc(RSTRING_LEN(data)), (unsigned int)RSTRING_LEN(data));
  memcpy(req->buf.base, RSTRING_PTR(data), RSTRING_LEN(data));
  
  uv_write(&req->req, rbuv_stream->uv_handle, &req->buf, 1, _uv_stream_on_write);
  
  return data;
}

void _uv_stream_on_connection(uv_stream_t *uv_server, int status) {
  VALUE server;
  rbuv_stream_t *rbuv_server;
  VALUE on_connection;
  
  RBUV_DEBUG_LOG("uv_server: %p, status: %d", uv_server, status);
  if (status == -1) {
    return;
  }
  
  server = (VALUE)uv_server->data;
  Data_Get_Struct(server, rbuv_stream_t, rbuv_server);
  on_connection = rbuv_server->cb_on_connection;
  
  RBUV_DEBUG_LOG_DETAIL("server: %ld, on_connection: %ld", server, on_connection);
  
  rb_funcall(on_connection, id_call, 1, server);
}

uv_buf_t _uv_alloc_buffer(uv_handle_t *handle, size_t suggested_size) {
  return uv_buf_init((char *)malloc(suggested_size), (unsigned int)suggested_size);
}

void _uv_stream_on_read(uv_stream_t *uv_stream, ssize_t nread, uv_buf_t buf) {
  VALUE stream;
  rbuv_stream_t *rbuv_stream;
  VALUE on_read;
  uv_err_t uv_err;
  VALUE error;

  RBUV_DEBUG_LOG("uv_stream: %p, nread: %ld", uv_stream, nread);

  stream = (VALUE)uv_stream->data;
  Data_Get_Struct(stream, rbuv_stream_t, rbuv_stream);
  on_read = rbuv_stream->cb_on_read;
  RBUV_DEBUG_LOG_DETAIL("stream: %ld, on_read: %ld", stream, on_read);

  if (nread > 0) {
    rb_funcall(on_read, id_call, 1, rb_str_new(buf.base, nread));
  } else {
    uv_err = uv_last_error(uv_default_loop());
    if (uv_err.code == UV_EOF) {
      error = rb_exc_new2(rb_eEOFError, "end of file reached");
    } else {
      error = rb_exc_new2(eRbuvError, uv_strerror(uv_err));
    }
    rb_funcall(on_read, id_call, 2, Qnil, error);
  }

  assert(buf.base);
  free(buf.base);
}

void _uv_stream_on_write(uv_write_t *uv_req, int status) {
  rbuv_write_req_t *rbuv_req;
  RBUV_DEBUG_LOG("req: %p, status: %d", uv_req, status);

  rbuv_req = RBUV_CONTAINTER_OF(uv_req, rbuv_write_req_t, req);

  free(rbuv_req->buf.base);
  free(rbuv_req);
  
  RBUV_CHECK_UV_RETURN(status);
}