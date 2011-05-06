package = "lua-yajl"
version = "1.3-1"
source = {
	url = "git://github.com/brimworks/lua-yajl.git",
	branch = "v1.3",
}
description = {
	summary = "Integrate the yajl JSON library with Lua.",
	homepage = "http://github.com/brimworks/lua-yajl",
	license = "MIT/X11",
}
external_dependencies = {
  YAJL = {
    header = "yajl/yajl_parse.h",
    library = "yajl",
  },
}
dependencies = {
	"lua >= 5.1",
}
build = {
	type = "builtin",
	modules = {
		yajl = {
			sources = { "lua_yajl.c" },
			libraries = { "yajl" },
		},
	},
}
