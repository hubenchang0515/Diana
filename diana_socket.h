/*
** File    : diana_socket.h
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

#ifndef DIANA_SOCKET_H
#define DIANA_SOCKET_H

#include <lualib.h>
#include <lauxlib.h>

int diana_socket_tcp(lua_State* L);
int diana_socket_udp(lua_State* L);
int diana_socket_bind(lua_State* L);
int diana_socket_connect(lua_State* L);
int diana_socket_listen(lua_State* L);
int diana_socket_accept(lua_State* L);

#endif // DIANA_SOCKET_H