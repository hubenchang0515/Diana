/*
** File    : diana_socket.c
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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "diana_io.h"
#include "diana_socket.h"

/* Index in Lua Registry */
static const int DIANA_SOCKET = 0;
void* DIANA_SOCKET_KEY = (void*)&DIANA_SOCKET;


/* private */
static void diana_socket_pack(lua_State* L,int fd, int domain, int type, int protocol);

/* execute while require("diana.socket") */
int luaopen_diana_socket(lua_State* L)
{
    /* Key of Lua Registry */
	lua_pushlightuserdata(L,DIANA_SOCKET_KEY);

    /* create a table to store diana.socket */
	lua_newtable(L); // diana.socket

    /* Bind functions */
    lua_pushcfunction(L,diana_socket_tcp);
	lua_setfield(L,-2,"tcp");
	lua_pushcfunction(L,diana_socket_udp);
	lua_setfield(L,-2,"udp");
	lua_pushcfunction(L,diana_socket_bind);
	lua_setfield(L,-2,"bind");
	lua_pushcfunction(L,diana_socket_connect);
	lua_setfield(L,-2,"connect");
	lua_pushcfunction(L,diana_socket_listen);
	lua_setfield(L,-2,"listen");
	lua_pushcfunction(L,diana_socket_accept);
	lua_setfield(L,-2,"accept");

	lua_pushcfunction(L,diana_io_read);
	lua_setfield(L,-2,"read");
	lua_pushcfunction(L,diana_io_write);
	lua_setfield(L,-2,"write");
	lua_pushcfunction(L,diana_io_close);
	lua_setfield(L,-2,"close");
	lua_pushcfunction(L,diana_io_string);
	lua_setfield(L,-2,"string");
	

    /* Set registry */
	lua_settable(L,LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L,DIANA_SOCKET_KEY);
	lua_gettable(L,LUA_REGISTRYINDEX);

    return 1;
	
}


/* Create a TCP socket */
int diana_socket_tcp(lua_State* L)
{
    int fd = socket(AF_INET,SOCK_STREAM,0);
	if(fd == -1)
	{
		lua_pushnil(L);
	}
	else
	{
    	diana_socket_pack(L,fd,AF_INET,SOCK_STREAM,0);
	}

    return 1;
}


/* Create a UDP socket */
int diana_socket_udp(lua_State* L)
{
	int fd = socket(AF_INET,SOCK_DGRAM,0);
	if(fd == -1)
	{
		lua_pushnil(L);
	}
	else
	{
		diana_socket_pack(L,fd,AF_INET,SOCK_DGRAM,0);
	}

	return 1;
}
#include <stdio.h>

/* Socket bind */
int diana_socket_bind(lua_State* L)
{
	struct sockaddr_in addr;
	lua_getfield(L,1,"__fd");
	int fd = luaL_checkinteger(L,-1);
	
	if(fd < 0)
	{
		lua_pushboolean(L,0);
	}
	else
	{
		lua_getfield(L,1,"__domain");
		addr.sin_family = luaL_checkinteger(L,-1);
		addr.sin_addr.s_addr = inet_addr(luaL_checkstring(L,2));
		addr.sin_port = htons(luaL_checkinteger(L,3));

		if(0 == bind( fd , (struct sockaddr*)&addr , sizeof(addr) ))
		{
			lua_pushboolean(L,1);
		}
		else
		{
			lua_pushboolean(L,0);
		}
	}
	
	return 1;
}

/* socket connect */
int diana_socket_connect(lua_State* L)
{
	struct sockaddr_in addr;
	lua_getfield(L,1,"__fd");
	int fd = luaL_checkinteger(L,-1);
	if(fd < 0)
	{
		lua_pushboolean(L,0);
	}
	else
	{
		lua_getfield(L,1,"__domain");
		addr.sin_family = luaL_checkinteger(L,-1);
		addr.sin_addr.s_addr = inet_addr(luaL_checkstring(L,2));
		addr.sin_port = htons(luaL_checkinteger(L,3));

		if(0 == connect( fd , (struct sockaddr*)&addr , sizeof(addr) ))
		{
			lua_pushboolean(L,1);
		}
		else
		{
			lua_pushboolean(L,0);
		}
	}

	return 1;
}


/* socket listen */
int diana_socket_listen(lua_State* L)
{
	lua_getfield(L,1,"__fd");
	int fd = luaL_checkinteger(L,-1);
	if(fd < 0)
	{
		lua_pushboolean(L,0);
	}
	else
	{
		int backlog = 0;
		if(lua_gettop(L) > 1)
		{
			backlog = luaL_checkinteger(L,2);
		}
		
		luaL_checkinteger(L,2);
		if(listen(fd,backlog) == 0)
		{
			lua_pushboolean(L,1);
		}
		else
		{
			lua_pushboolean(L,0);
		}
	}
	
	return 1;
}


/* socket accept */
int diana_socket_accept(lua_State* L)
{
	lua_getfield(L,1,"__fd");
	int fd = luaL_checkinteger(L,-1);
	if(fd < 0)
	{
		lua_pushnil(L);
		return 1;
	}
	else
	{
		struct sockaddr_in addr;
		socklen_t len = sizeof(addr);
		int rfd = accept(fd,(struct sockaddr*)&addr , &len);
		if(rfd == -1)
		{
			lua_pushnil(L);
			return 1;
		}
		else
		{
			lua_getfield(L,1,"__domain");
			int domain = luaL_checkinteger(L,-1);
			lua_getfield(L,1,"__type");
			int type = luaL_checkinteger(L,-1);
			lua_getfield(L,1,"__protocol");
			int protocol = luaL_checkinteger(L,-1);
			diana_socket_pack(L,rfd,domain,type,protocol);
			lua_pushstring(L,inet_ntoa(addr.sin_addr));
			lua_pushinteger(L,ntohs(addr.sin_port));

			return 3;
		}
	}	
}


/*======================================*/
static void diana_socket_pack(lua_State* L,int fd, int domain, int type, int protocol)
{
	lua_newtable(L); // as return value
	lua_pushinteger(L,fd);
	lua_setfield(L,-2,"__fd"); // t['__fd'] = fd
    lua_pushinteger(L,domain);
	lua_setfield(L,-2,"__domain"); // t['__domain'] = domain
	lua_pushinteger(L,type);
	lua_setfield(L,-2,"__type"); // t['__type'] = type
    lua_pushinteger(L,protocol);
	lua_setfield(L,-2,"__protocol"); // t['__protocol'] = protocol
	
	lua_newtable(L); // as metatable
	lua_pushlightuserdata(L,DIANA_SOCKET_KEY);
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