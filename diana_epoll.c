/*
** File    : diana_epoll.c
** Author  : Plan C
** Project : https://github.com/hubenchang0515/Diana
** Blog    : blog.kurukurumi.com
** E-main  : hubenchang0515@outlook.com
**

MIT License

Copyright (c) 2017 Plan C

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <sys/epoll.h>
#include <lualib.h>
#include <lauxlib.h>
#include "diana_io.h"



/* Index in Lua Registry */
static const int DIANA_EPOLL = 0;
void* DIANA_EPOLL_KEY = (void*)&DIANA_EPOLL;

static int diana_epoll_create(lua_State* L);
static int diana_epoll_control(lua_State* L);
static int diana_epoll_wait(lua_State* L);

static void diana_epoll_pack(lua_State* L,int fd);
static void diana_epoll_io_pack(lua_State* L,int fd);

/* functions of epoll */
static luaL_Reg functions[] = 
{
	{ "create"  , diana_epoll_create, },
	{ "control" , diana_epoll_control, },
	{ "wait"    , diana_epoll_wait, },
	{ "close"   , diana_io_close, },
	{ "error"   , diana_io_error, },
	{ "equal"   , diana_io_equal, },
	{ "string"  , diana_io_string, },
	{NULL,NULL}
};

/* values of epoll */
typedef struct int_Reg
{
	const char* name;
	int value;
}int_Reg;

static int_Reg values[] = 
{
	{"CTL_ADD" , EPOLL_CTL_ADD,},
	{"CTL_DEL" , EPOLL_CTL_DEL,},
	{"CTL_MOD" , EPOLL_CTL_MOD,},
	{"IN"  , EPOLLIN,},
	{"OUT" , EPOLLOUT,},
	{"PRI" , EPOLLPRI,},
	{"ERR" , EPOLLERR,},
	{"HUP" , EPOLLHUP,},
	{"ET" , EPOLLET,},
	{"ONESHOT" , EPOLLONESHOT,},
	{NULL,0}
};

/* execute in require('diana.epoll') */
int luaopen_diana_epoll(lua_State* L)
{
	/* Key of Lua Registry */
	lua_pushlightuserdata(L,DIANA_EPOLL_KEY);

	/* create a table to store diana.epoll */
	lua_newtable(L); // diana.epoll

	/* functions */
	for(size_t i = 0; functions[i].name != NULL; i++)
	{
		lua_pushcfunction(L,functions[i].func);
		lua_setfield(L,-2,functions[i].name);
	}

	/* values */
	for(size_t i = 0; values[i].name != NULL; i++)
	{
		lua_pushinteger(L,values[i].value);
		lua_setfield(L,-2,values[i].name);
	}

	/* Set registry */
	lua_settable(L,LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L,DIANA_EPOLL_KEY);
	lua_gettable(L,LUA_REGISTRYINDEX);

	return 1;
    
}


/* create epoll */
static int diana_epoll_create(lua_State* L)
{
    int size = luaL_checkinteger(L,1);
    int epollfd = epoll_create(size);
	if(epollfd == -1)
	{
		lua_pushnil(L);
	}
	else
	{
		diana_epoll_pack(L,epollfd);
	}
	
	return 1;
}


/* control epoll */
static int diana_epoll_control(lua_State* L)
{
	lua_getfield(L,1,"__fd");
	int epollfd = luaL_checkinteger(L,-1);
	lua_getfield(L,3,"__fd");
	int fd = luaL_checkinteger(L,-1);
	int opt = luaL_checkinteger(L,2);
	struct epoll_event event;
	event.events = luaL_checkinteger(L,4);
	event.data.fd = fd;
	lua_pushboolean(L,-1 != epoll_ctl(epollfd,opt,fd,&event));

	return 1;
}


/* block wait */
static int diana_epoll_wait(lua_State* L)
{
	int timeout = -1;
	if(lua_gettop(L) > 1)
	{
		printf("%d\n",lua_gettop(L) );
		timeout = luaL_checkinteger(L,2);
	}

	lua_getfield(L,1,"__fd");
	int epollfd = luaL_checkinteger(L,-1);

	struct epoll_event event;
	int state = epoll_wait(epollfd,&event,1,timeout);
	if(-1 == state || 0 == state) // -1 : failed  , 0 : timeout
	{
		lua_pushnil(L);
		return 1;
	}
	else
	{
		diana_epoll_io_pack(L,event.data.fd);
		lua_pushinteger(L,event.events);
		return 2;
	}
}

/*=================================*/
/* pack fd and set metatable */
static void diana_epoll_pack(lua_State* L,int fd) // fd of epoll
{
	lua_newtable(L); // as return value
	lua_pushinteger(L,fd);
	lua_setfield(L,-2,"__fd"); // t['__fd'] = fd
	
	lua_newtable(L); // as metatable
	lua_pushlightuserdata(L,DIANA_EPOLL_KEY);
	lua_gettable(L,LUA_REGISTRYINDEX);
	lua_setfield(L,-2,"__index");
	lua_pushcfunction(L,diana_io_close);
	lua_setfield(L,-2,"__gc"); // gc
	lua_pushcfunction(L,diana_io_equal);
	lua_setfield(L,-2,"__eq");
	lua_pushcfunction(L,diana_io_string);
	lua_setfield(L,-2,"__tostring");

	lua_setmetatable(L,-2);
}

static void diana_epoll_io_pack(lua_State* L,int fd) // fd returned by epoll_wait
{
    lua_newtable(L); // as return value
	lua_pushinteger(L,fd);
	lua_setfield(L,-2,"__fd"); // t['__fd'] = fd

    lua_newtable(L); // as metatable

	lua_newtable(L); // as __index of metatable
    lua_pushcfunction(L,diana_io_read);
	lua_setfield(L,-2,"read");
	lua_pushcfunction(L,diana_io_write);
	lua_setfield(L,-2,"write");
	lua_pushcfunction(L,diana_io_error);
	lua_setfield(L,-2,"error");
	lua_pushcfunction(L,diana_io_equal);
	lua_setfield(L,-2,"equal");
	lua_pushcfunction(L,diana_io_string);
	lua_setfield(L,-2,"string");
    lua_setfield(L,-2,"__index");  // set __index

    lua_pushcfunction(L,diana_io_equal);
	lua_setfield(L,-2,"__eq");
	lua_pushcfunction(L,diana_io_string);
	lua_setfield(L,-2,"__tostring");
	lua_setmetatable(L,-2);
}
