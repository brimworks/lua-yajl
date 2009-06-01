_pwd := $(pwd)

include $(make-common.dir)/tool/cc.mk
include $(make-common.dir)/tool/lua.mk
include $(make-common.dir)/layout.mk

_lib  := $(lua.lib.dir)/yajl.so
_objs := $(call cc.c.to.o,$(addprefix $(_pwd)/, \
    lua_yajl.c \
))

all: | $(_lib)
$(_lib): cc.libs += lua yajl
$(_lib): cc.objs := $(_objs)
$(_lib): $(_objs)
	$(cc.so.rule)

# How to run lua_yajl tests:
.PHONY: lua_yajl.test
test: | lua_yajl.test

lua ?= lua
lua_yajl.test: | $(_lib)
lua_yajl.test: lua.path += $(_pwd)
lua_yajl.test: $(wildcard $(_pwd)/test*)
	@mkdir -p $(tmp.dir)
	cd $(tmp.dir); for t in $<; do \
		echo "TESTING: $$t"; \
		env -i $(lua.run) $(lua) $$t; \
	done
