#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "lodepng.h"

void encode(const char* filename, const unsigned char* image, unsigned width, unsigned height) {
	unsigned error = lodepng_encode32_file(filename, image, width, height);
	if (error) { 
		fprintf(stderr, "error %u: %s\n", error, lodepng_error_text(error));
	}
}

int main(int argc, char ** argv) {
	int height, width;
	if (argc < 3) {
		height = 512;
		width = 512;
	} else {
		height = strtol(argv[1], NULL, 10);
		width = strtol(argv[2], NULL, 10);
	}

	lua_State *L;
	L = luaL_newstate();
	luaL_openlibs(L);
	int status = luaL_dofile(L, "script.lua");

	if (status) {
		fprintf(stderr, "File didn't load %s\n",
				lua_tostring(L, -1));
		exit(1);
	}

	uint8_t *buf = malloc(width * height * 4 * sizeof(uint8_t));
	for (int i = 0; i < width * height; ++i) {
		lua_getglobal(L, "foo");
		lua_pushnumber(L, buf[i]);
		lua_pushnumber(L, i % width);
		lua_pushnumber(L, i / width);
		if (lua_pcall(L, 3, 1, 0) != 0) {
			fprintf(stderr, "error running function 'foo': %s\n", lua_tostring(L, -1));
			exit(1);
		}
		if (!lua_isnumber(L, -1)) {
			fprintf(stderr, "function f must return number\n");
			exit(1);
		}
		int val = lua_tonumber(L, -1);
		buf[i * 4 + 0] = val;
		buf[i * 4 + 1] = val;
		buf[i * 4 + 2] = val;
		buf[i * 4 + 3] = 255;
		lua_pop(L, 1);
	}
	encode("out.png", buf, width, height);


	lua_close(L);
	return 0;
}
