/*
** File    : diana_io.c
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

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "fdpopen.h"
#include "diana_io.h"

/* Index in Lua Registry */
static const int DIANA_IO = 0;
void* DIANA_IO_KEY = (void*)&DIANA_IO;


/* private */
static void diana_io_pack(lua_State* L,int fd);

/* execute while require("diana.io") */
int luaopen_diana_io(lua_State* L)
{
	/* Key of Lua Registry */
	lua_pushlightuserdata(L,DIANA_IO_KEY);

	/* create a table to store diana.io */
	lua_newtable(L); // diana.io
	
	/* Set functions */
	lua_pushcfunction(L,diana_io_open);
	lua_setfield(L,-2,"open");
	lua_pushcfunction(L,diana_io_popen);
	lua_setfield(L,-2,"popen");
	lua_pushcfunction(L,diana_io_close);
	lua_setfield(L,-2,"close");
	lua_pushcfunction(L,diana_io_read);
	lua_setfield(L,-2,"read");
	lua_pushcfunction(L,diana_io_write);
	lua_setfield(L,-2,"write");
	lua_pushcfunction(L,diana_io_seek);
	lua_setfield(L,-2,"seek");
	lua_pushcfunction(L,diana_io_seek);
	lua_setfield(L,-2,"equal");
	lua_pushcfunction(L,diana_io_string);
	lua_setfield(L,-2,"string");
	
	/* Set registry */
	lua_settable(L,LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L,DIANA_IO_KEY);
	lua_gettable(L,LUA_REGISTRYINDEX);
	
	return 1;
}





/*
 * Open a file
 *
 * Mode :  r  - readonly
 *         a  - writeonly , allways write to end , create if file not exist
 *         w  - writeonly , clear exist data ,create if file not exist
 *         r+ - read and write
 *         a+ - read and write , allways write to end , create if file not exist
 *         w+ - read and write , clear exist data , create if file not exist
 *
 */
int diana_io_open(lua_State* L)
{
	int argc = lua_gettop(L); // 栈顶的索引，即参数个数
	const char* file = luaL_checkstring(L,1);
	int oflag = O_RDONLY;
	
	if(argc > 1)
	{
		const char* mode = luaL_checkstring(L,2);
		if(!strcmp("r",mode))  // read mode
		{
			oflag = O_RDONLY;
		}
		else if(!strcmp("w",mode)) // write mode 
		{
			oflag = O_WRONLY | O_TRUNC | O_CREAT;
		}
		else if(!strcmp("a",mode)) // read and write 
		{
			oflag = O_WRONLY | O_APPEND | O_CREAT;
		}
		else if(!strcmp("r+",mode))
		{
			oflag = O_RDWR;
		}
		else if(!strcmp("w+",mode))
		{
			oflag = O_RDWR | O_TRUNC | O_CREAT;
		}
		else if(!strcmp("a+",mode))
		{
			oflag = O_RDWR | O_APPEND | O_CREAT;
		}
		else
		{
			lua_pushstring(L,"Unknow open mode.");
			lua_error(L);
		}
	}
	
	int fd = open(file, oflag, 0664);
	if(fd != -1)
	{
		diana_io_pack(L,fd);
	}
	else
	{
		lua_pushnil(L);
	}
	
	return 1;
}

/*
 * popen a command
 */
int diana_io_popen(lua_State* L)
{
	const char* cmd  = luaL_checkstring(L,1);
	const char* type = luaL_checkstring(L,2);
	int fd = fdpopen(cmd,type);
	if(fd != -1)
	{
		diana_io_pack(L,fd);
		lua_pushboolean(L,1);
		lua_setfield(L,-2,"__bypopen"); // opened by popen
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

/*
 * Close file
 */
int diana_io_close(lua_State* L)
{
	lua_getfield(L,1,"__fd");
	int fd = luaL_checkinteger(L,-1);
	lua_getfield(L,1,"__bypopen");
	int bypopen = lua_toboolean(L,-1);
	lua_pushinteger(L,-1);
	lua_setfield(L,1,"__fd");
	if(bypopen)  // fd is opened by fdpopen
	{
		lua_pushinteger(L,fdpclose(fd));
	}
	else
	{
		lua_pushinteger(L,close(fd));
	}
	return 1;
}


/*
 * Read file
 *
 * read all file or n bytes( n is 2nd arg )
 *
 * return string and bytes read
 */
int diana_io_read(lua_State* L)
{
	lua_getfield(L,1,"__fd");
	int fd = luaL_checkinteger(L,-1);
	lua_pop(L,1);

#ifndef BUFFER_SIZE
#	define BUFFER_SIZE 1024

	char buffer[BUFFER_SIZE+1];
	ssize_t len = BUFFER_SIZE; // length read once
	int number = 0; // number of strings pushed
	int length = 0; // length total
	
	if(lua_gettop(L) > 1)
	{
		int bytes = luaL_checkinteger(L,2);
		while( bytes > BUFFER_SIZE && len == BUFFER_SIZE)
		{			
			len = read(fd,buffer,BUFFER_SIZE);
			buffer[len] = 0;
			bytes -= len;
			length += len;
			lua_pushstring(L,buffer);
			number++;
		}
		if(bytes > 0)
		{
			len = read(fd,buffer,bytes);
			buffer[len] = 0;
			lua_pushstring(L,buffer);
			length += len;
			number++;
		}
		
	}
	else
	{
		do
		{
			len = read(fd,buffer,BUFFER_SIZE); 
			buffer[len] = 0;
			lua_pushstring(L,buffer);
			length += len;
			number++;
		}while(len == BUFFER_SIZE);
	}

#	undef BUFFER_SIZE
#endif
	
	lua_concat(L,number);// concat all strings
	lua_pushinteger(L,length);
	
	return 2;
	
}


/*
 * Write file
 */
int diana_io_write(lua_State* L)
{
	const char* data = luaL_checkstring(L,2);
	lua_getfield(L,1,"__fd");
	int fd = luaL_checkinteger(L,-1);
	
	ssize_t len = write(fd,data,strlen(data));
	lua_pushinteger(L,len);
	
	return 1;
}

/*
 * Seek file
 */
int diana_io_seek(lua_State* L)
{
	const char* from = "begin";
	off_t offset = 0; // not seek default : return current position
	int fd;
	switch(lua_gettop(L))
	{
	default: // >= 3
		from = luaL_checkstring(L,3);
	case 2: 
		offset = luaL_checkinteger(L,2);
	case 1:
		lua_getfield(L,1,"__fd");
		fd = luaL_checkinteger(L,-1);
	}
	
	/* =0 : from current
	   >0 : from begin
	   <0 : from end
	  */
	int fromwhere = SEEK_SET;
	if(!strcmp(from,"current"))
	{
		fromwhere = SEEK_CUR;
	}
	else if(!strcmp(from,"begin"))
	{
		fromwhere = SEEK_SET;
	}
	else if(!strcmp(from,"end"))
	{
		fromwhere = SEEK_END;
	}
	else
	{
		lua_pushstring(L,"Unknow from where.");
		lua_error(L);
	}
	
	off_t pos = lseek(fd,offset,fromwhere);
	lua_pushinteger(L,pos);
	
	return 1;
}

/*
 * compare two table of 
 */
int diana_io_equal(lua_State* L)
{
	lua_getfield(L,1,"__fd");
	int fd1 = luaL_checkinteger(L,-1);
	
	lua_getfield(L,2,"__fd");
	int fd2 = luaL_checkinteger(L,-1);
	
	lua_pushboolean(L,fd1 == fd2);
	
	return 1;
}

/*
 * Translate to string
 */
int diana_io_string(lua_State* L)
{
	lua_getfield(L,1,"__fd");
	int fd = luaL_checkinteger(L,-1);
	lua_pushfstring(L,"file: %d",fd);

	return 1;
}




/*======================================*/
static void diana_io_pack(lua_State* L,int fd)
{
	lua_newtable(L); // as return value
	lua_pushinteger(L,fd);
	lua_setfield(L,-2,"__fd"); // t['__fd'] = fd
	
	lua_newtable(L); // as metatable
	lua_pushlightuserdata(L,DIANA_IO_KEY);
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

