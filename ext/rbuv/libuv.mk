DEPS_DIR := $(srcdir)/../../deps
LIBUV_DIR := $(DEPS_DIR)/libuv/

INCFLAGS += -I"$(LIBUV_DIR)/include"

LIBUV_A = "$(LIBUV_DIR)/libuv.a"

$(OBJS): $(LIBUV_A)

OBJS += $(LIBUV_A)
$(LIBUV_A): $(LIBUV_DIR)
	$(MAKE) -C "$(LIBUV_DIR)"
