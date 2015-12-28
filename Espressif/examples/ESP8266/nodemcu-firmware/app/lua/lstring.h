/*
** $Id: lstring.h,v 1.43.1.1 2007/12/27 13:02:25 roberto Exp $
** String table (keep all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#ifndef lstring_h
#define lstring_h


#include "lgc.h"
#include "lobject.h"
#include "lstate.h"


#define sizestring(s) (sizeof(union TString)+(luaS_isreadonly(s) ? sizeof(char **) : ((s)->len+1)*sizeof(char)))

#define sizeudata(u)	(sizeof(union Udata)+(u)->len)

#define luaS_new(L, s)	(luaS_newlstr(L, s, c_strlen(s)))
#define luaS_newro(L, s)  (luaS_newrolstr(L, s, c_strlen(s)))
#define luaS_newliteral(L, s)  (luaS_newlstr(L, "" s, \
                                  (sizeof(s)/sizeof(char))-1))

#define luaS_fix(s)	l_setbit((s)->tsv.marked, FIXEDBIT)
#define luaS_readonly(s) l_setbit((s)->tsv.marked, READONLYBIT)
#define luaS_isreadonly(s) testbit((s)->marked, READONLYBIT)

LUAI_FUNC void luaS_resize (lua_State *L, int newsize);
LUAI_FUNC Udata *luaS_newudata (lua_State *L, size_t s, Table *e);
LUAI_FUNC TString *luaS_newlstr (lua_State *L, const char *str, size_t l);
LUAI_FUNC TString *luaS_newrolstr (lua_State *L, const char *str, size_t l);

#endif
