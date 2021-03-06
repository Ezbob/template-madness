// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <iostream>
#include <functional>
#include <cstddef>
#include <array>

const int STACK_TOP = -1;

bool LuaCheckError(lua_State *s, int return_code) {
    if (return_code != LUA_OK) {
        std::string errormsg = lua_tostring(s, -1);
        std::cout << errormsg << std::endl;
        return false;
    }
    return true;
}

/*
 * RAII for a lua state
 */
class LuaRAIIState {

public:
    LuaRAIIState() {
        m_state = luaL_newstate();
    }

    ~LuaRAIIState() {
        lua_close(m_state);
    }

    lua_State *ptr() {
        return m_state;
    }

    operator lua_State *() {
        return m_state;
    }

    bool checkError(int return_code) {
        if (return_code != LUA_OK) {
            std::string errormsg = lua_tostring(m_state, -1);
            std::cout << errormsg << std::endl;
            return false;
        }
        return true;
    }

    void requestGlobal(const std::string name) {
        lua_getglobal(m_state, name.c_str());
    }

    void requestGlobal(const char *name) {
        lua_getglobal(m_state, name);
    }

    void addGlobal(const std::string name) {
        lua_setglobal(m_state, name.c_str());
    }

    void addGlobal(const char *name) {
        lua_setglobal(m_state, name);
    }

    void push(double val) {
        lua_pushnumber(m_state, val);
    }

    void push(int val) {
        lua_pushnumber(m_state, val);
    }

    void push(bool val) {
        lua_pushboolean(m_state, val);
    }

    void push(const char *val) {
        lua_pushstring(m_state, val);
    }
    
    void push(const std::string val) {
        lua_pushstring(m_state, val.c_str());
    }

    void push(int (*function)(lua_State *)) {
        lua_pushcfunction(m_state, function);
    }

    void push(void) {
        lua_pushnil(m_state);
    }

    void pushCopyOf(int index) {
        lua_pushvalue(m_state, index);
    }

    bool hasNumber(int index = STACK_TOP) {
        return lua_isnumber(m_state, index);
    }

    int openTable() {
        lua_newtable(m_state);
        int tableref = lua_gettop(m_state);
        return tableref;
    }

    template<typename T>
    void addTableMember(int tableIndex, const std::string name, T value) {
        push(name);
        push(value);
        lua_settable(m_state, tableIndex);
    }

    template<typename T>
    void addTableMember(int tableIndex, const char *name, T value) {
        push(name);
        push(value);
        lua_settable(m_state, tableIndex);
    }

    double getNumber(int index = STACK_TOP) {
        return (double) lua_tonumber(m_state, index);
    }

    bool hasTable(int index = STACK_TOP) {
        return lua_istable(m_state, index);
    }

    void requestFromTable(int table_index = (STACK_TOP - 1)) {
        lua_gettable(m_state, table_index);
    }

    bool hasString(int index = STACK_TOP) {
        return lua_isstring(m_state, index);
    }

    std::string getString(int index = STACK_TOP) {
        return lua_tostring(m_state, index);
    }

    bool hasBool(int index = STACK_TOP) {
        return lua_isboolean(m_state, index);
    }

    bool hasFunction(int index = STACK_TOP) {
        return lua_isfunction(m_state, index);
    }

    bool getBool(int index = STACK_TOP) {
        return lua_toboolean(m_state, index);
    }

    void pop(int number_of_elements = 1) {
        lua_pop(m_state, number_of_elements);
    }

    int stackSize() {
        return lua_gettop(m_state);
    }

    void clearStack() {
        lua_pop(m_state, lua_gettop(m_state));
    }

    void registerFunction(std::string functionName, int (*function)(lua_State *)) {
        lua_register(m_state, functionName.c_str(), function);
    }

    void registerFunction(const char *functionName, int (*function)(lua_State *)) {
        lua_register(m_state, functionName, function);
    }

    bool callFunction(int numberOfInputArgs, int numberOfReturnValues, int errorHandlerIndex = 0) {
        return checkError(lua_pcall(m_state, numberOfInputArgs, numberOfReturnValues, errorHandlerIndex));
    }

private:
    lua_State *m_state;
};

struct LuaFunctionArg {
    enum ArgType {
        STRING,
        INT,
        DOUBLE,
        BOOL
    } type;
    union ArgValue {
        const char *strval;
        int intval;
        double doubleval;
        bool boolval;
    } value;

    LuaFunctionArg(const char *str) : type(STRING) {
        value.strval = str;
    }

    LuaFunctionArg(int intval) : type(INT) {
        value.intval = intval;
    }

    LuaFunctionArg(double dval) : type(DOUBLE) {
        value.doubleval = dval;
    }

    LuaFunctionArg(bool bval) : type(BOOL) {
        value.boolval = bval;
    }

    LuaFunctionArg(LuaFunctionArg::ArgType t) : type(t) {
        switch (t) {
            case LuaFunctionArg::ArgType::STRING:
                value.strval = "";
                break;
            case LuaFunctionArg::ArgType::INT:
                value.intval = 0;
                break;
            case LuaFunctionArg::ArgType::DOUBLE:
                value.doubleval = 0.0;
                break;
            case LuaFunctionArg::ArgType::BOOL:
                value.boolval = false;
                break;
            default:
                break;
        }
    }

    void getFromStack(lua_State *L, int stack_index=STACK_TOP) {
         switch(type) {
            case LuaFunctionArg::STRING:
                if (lua_isstring(L, stack_index)) {
                    value.strval = lua_tostring(L, stack_index);
                }
                break;
            case LuaFunctionArg::INT:
                if (lua_isinteger(L, stack_index)) {
                    value.intval = lua_tointeger(L, stack_index);
                }
                break;
            case LuaFunctionArg::DOUBLE:
                if (lua_isnumber(L, stack_index)) {
                    value.doubleval = (double) lua_tonumber(L, stack_index);
                }
                break;
            case LuaFunctionArg::BOOL:
                if (lua_isboolean(L, stack_index)) {
                    value.boolval = lua_toboolean(L, stack_index);
                }
                break;
        }
    }

    void pushToStack(lua_State *L) {
        switch(type) {
            case LuaFunctionArg::STRING:
                lua_pushstring(L, value.strval);
                break;
            case LuaFunctionArg::INT:
                lua_pushinteger(L, value.intval);
                break;
            case LuaFunctionArg::DOUBLE:
                lua_pushnumber(L, value.doubleval);
                break;
            case LuaFunctionArg::BOOL:
                lua_pushboolean(L, value.boolval);
                break;
        }
    }
};


template<int nArgs>
using LuaFunctionArgs = std::array<LuaFunctionArg, nArgs>; 

template<int nInputArgs = 0, int nOutputArgs = 0, int faultHandlerIndex = 0>
class LuaTableFunction {

public:
    bool operator() (LuaFunctionArgs<nOutputArgs> *outputArgs = nullptr, LuaFunctionArgs<nInputArgs> *inputArgs = nullptr) {
        lua_pushstring(m_state, m_name);
        lua_gettable(m_state, (STACK_TOP - 1));
        if ( lua_isfunction(m_state, STACK_TOP) ) {

            if ( inputArgs != nullptr ) {
                for (auto l : (*inputArgs)) {
                    l.pushToStack(m_state);
                }
            }

            if ( LuaCheckError(m_state, lua_pcall(m_state, m_input, m_output, m_faultHandler_index)) ) {
                if ( outputArgs != nullptr ) {
                    if ( nOutputArgs != outputArgs->size() ) {
                        throw std::out_of_range("Error: Output arguments size mismatch");
                    }

                    int i = 0;
                    for (LuaFunctionArg &arg : (*outputArgs)) {
                        arg.getFromStack(m_state, STACK_TOP - i);
                        ++i;
                    }
                }
            } else {
                lua_pop(m_state, 1);
                return false;
            }
        } else {
            lua_pop(m_state, 1);
            return false;
        }
        lua_pop(m_state, m_output + 1);
        return true;
    }

    void setMembers(lua_State *s, const char *name, const char *table_name) {
        m_state = s;
        m_name = name;
        m_table_name = table_name;
    }

    constexpr int getInputNArgs() const {
        return nInputArgs;
    }

    constexpr int getOutputNArgs() const {
        return nOutputArgs;
    }

private:
    int m_input = nInputArgs;
    int m_output = nOutputArgs;
    int m_faultHandler_index = faultHandlerIndex;
    const char *m_name;
    const char *m_table_name;
    lua_State *m_state;
};

class LuaTableConverter {

private:
    lua_State *m_state;
    int m_table_index = (STACK_TOP - 1);
    const char *m_table_name = "";

public:
    bool readFromGlobal(lua_State *luaState, const char *tableName) {
        m_table_name = tableName;
        m_state = luaState;

        lua_getglobal(m_state, m_table_name);
        if ( lua_istable(m_state, STACK_TOP) ) {
            LuaTableMappings();
            return true;
        }
        return false;
    }

    explicit LuaTableConverter() = default;

protected:
    virtual void LuaTableMappings() {}

    void map(std::string &s, const char *luaName) {

        lua_pushstring(m_state, luaName);
        lua_gettable(m_state, (STACK_TOP - 1));

        if ( lua_isstring(m_state, STACK_TOP) ) {
            s = lua_tostring(m_state, STACK_TOP);
        }

        lua_pop(m_state, 1);
    }

    void map(const char *&s, const char *luaName) {

        lua_pushstring(m_state, luaName);
        lua_gettable(m_state, (STACK_TOP - 1));

        if ( lua_isstring(m_state, STACK_TOP) ) {
            s = lua_tostring(m_state, STACK_TOP);
        }

        lua_pop(m_state, 1);
    }

    template<typename T>
    void map(T &s, const char *luaName) {

        lua_pushstring(m_state, luaName);
        lua_gettable(m_state, (STACK_TOP - 1));

        if ( lua_isnumber(m_state, STACK_TOP) ) {
            s = lua_tonumber(m_state, STACK_TOP);
        }

        lua_pop(m_state, 1);
    }

    void required_map(std::string &s, const char *luaName) {

        lua_pushstring(m_state, luaName);
        lua_gettable(m_state, (STACK_TOP - 1));

        if ( lua_isstring(m_state, STACK_TOP) ) {
            s = lua_tostring(m_state, STACK_TOP);
            lua_pop(m_state, 1);
        } else {
            lua_pop(m_state, 1);
            throw std::out_of_range(luaName);
        }
    }

    void required_map(const char *&s, const char *luaName) {

        lua_pushstring(m_state, luaName);
        lua_gettable(m_state, (STACK_TOP - 1));

        if ( lua_isstring(m_state, STACK_TOP) ) {
            s = lua_tostring(m_state, STACK_TOP);
            lua_pop(m_state, 1);
        } else {
            lua_pop(m_state, 1);
            throw std::out_of_range(luaName);
        }

    }

    template<typename T>
    void required_map(T &s, const char *luaName) {

        lua_pushstring(m_state, luaName);
        lua_gettable(m_state, (STACK_TOP - 1));

        if ( lua_isnumber(m_state, STACK_TOP) ) {
            s = lua_tonumber(m_state, STACK_TOP);
            lua_pop(m_state, 1);
        } else {
            lua_pop(m_state, 1);
            throw std::out_of_range(luaName);
        }
    }

    template<int inputArgs, int outputArgs>
    void required_map(LuaTableFunction<inputArgs, outputArgs> &s, const char *luaName) {

        lua_pushstring(m_state, luaName);
        lua_gettable(m_state, (STACK_TOP - 1));

        if ( lua_isfunction(m_state, STACK_TOP) ) {
            s.setMembers(m_state, luaName, m_table_name);
            lua_pop(m_state, 1);
        } else {
            lua_pop(m_state, 1);
            throw std::out_of_range(luaName);
        }
    }


};
