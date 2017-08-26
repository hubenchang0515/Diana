#include <lualib.h>
#include <lauxlib.h>
#include <signal.h>

static lua_State* diana_signal_L;
static lua_State* luaL;

static int diana_signal_set(lua_State* L);
static void diana_signal_callback(int sig);

struct sig_reg
{
    int  sig;
    const char* name;
};

struct sig_reg sigs[] = 
{
    { SIGABRT , "SIGABRT" },
    { SIGFPE  , "SIGFPE"  },
    { SIGILL  , "SIGILL"  },
    { SIGINT  , "SIGINT"  },
    { SIGSEGV , "SIGSEGV" },
    { SIGTERM , "SIGTERM" },
    { 0,NULL}
};

int luaopen_diana_signal(lua_State* L)
{
    luaL = L;
    diana_signal_L = luaL_newstate();

    lua_newtable(L); // diana.signal

    lua_pushcfunction(L,diana_signal_set);
    lua_setfield(L,-2,"set");
    for(size_t i = 0; sigs[i].name != NULL; i++)
    {
        lua_pushinteger(L,sigs[i].sig);
        lua_setfield(L,-2,sigs[i].name);
    }

    return 1;
}

static int diana_signal_set(lua_State* L)
{

    int sig = luaL_checkinteger(L,1);
    int len = lua_gettop(diana_signal_L);


    /* Ensure sig is a valid index */
    while(len < sig)
    {
        lua_pushinteger(diana_signal_L,0);
        len++;
    }

    /* move a lua function to diana_signal_L */
    lua_xmove(L,diana_signal_L,1);
    /* copy the lua function to the index sig */
    lua_copy(diana_signal_L,-1,sig);
    
    signal(sig,diana_signal_callback);
    
    return 0;
}


static void diana_signal_callback(int sig)
{
    /* create a valid index to copy function */
    lua_pushinteger(diana_signal_L,0);

    /* copy the function to index -1 */
    lua_copy(diana_signal_L,sig,-1);

    /* move the function to luaL */
    lua_xmove(diana_signal_L,luaL,1);

    /* push sig as argument */
    lua_pushinteger(luaL,sig);
    
    /* call */
    lua_call(luaL,1,0);
}
