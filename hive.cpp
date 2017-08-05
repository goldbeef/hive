/*
** repository: https://github.com/trumanzhao/luna
** trumanzhao, 2017-05-13, trumanzhao@foxmail.com
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <locale>
#include <stdint.h>
#include <signal.h>
#include <err.h>
#include "hive.h"
#include "tools.h"

#ifdef _MSC_VER
int daemon(int nochdir, int noclose) { return 0; }
#endif

hive_app* g_app = nullptr;

static void on_signal(int signo)
{
    if (g_app)
    {
        g_app->set_signal(signo);
    }
}

EXPORT_CLASS_BEGIN(hive_app)
EXPORT_LUA_FUNCTION(get_version)
EXPORT_LUA_FUNCTION(get_file_time)
EXPORT_LUA_FUNCTION(get_time_ms)
EXPORT_LUA_FUNCTION(get_time_ns)
EXPORT_LUA_FUNCTION(sleep_ms)
EXPORT_LUA_FUNCTION(daemon)
EXPORT_LUA_FUNCTION(register_signal)
EXPORT_LUA_FUNCTION(default_signal)
EXPORT_LUA_FUNCTION(ignore_signal)
EXPORT_LUA_INT64(m_signal)
EXPORT_LUA_INT(m_reload_time)
EXPORT_LUA_INT(m_archive_buffer_size)
EXPORT_LUA_INT(m_archive_lz_threshold)
EXPORT_LUA_STD_STR_R(m_entry)
EXPORT_CLASS_END()

int hive_app::get_version(lua_State* L)
{
    lua_pushinteger(L, MAJOR_VERSION_NUMBER);
    lua_pushinteger(L, MINOR_VERSION_NUMBER);
    lua_pushinteger(L, REVISION_NUMBER);
    return 3;
}

time_t hive_app::get_file_time(const char* file_name)
{
    return ::get_file_time(file_name);
}

int64_t hive_app::get_time_ms()
{
    return ::get_time_ms();
}

int64_t hive_app::get_time_ns()
{
    return ::get_time_ns();
}

void hive_app::sleep_ms(int ms)
{
    ::sleep_ms(ms);
}

int hive_app::daemon(int nochdir, int noclose)
{
    return ::daemon(nochdir, noclose);
}

void hive_app::register_signal(int n)
{
    signal(n, on_signal);
}

void hive_app::default_signal(int n)
{
    signal(n, SIG_DFL);
}

void hive_app::ignore_signal(int n)
{
    signal(n, SIG_IGN);
}

void hive_app::set_signal(int n)
{
    uint64_t mask = 1;
    mask <<= n;
    m_signal |= mask;
}

static const char* g_sandbox = u8R"__(
hive.files = {};
hive.meta = {__index=function(t, k) return _G[k]; end};
hive.print = function(...) end; --do nothing

local try_load = function(filename, node)
    local real_path = node.path;
    local trunk, msg = loadfile(real_path, "bt", node.env);
    if not trunk then
        hive.print(string.format("load file: %s ... ... [failed]", filename));
        hive.print(msg);
        return;
    end

    local ok, err = pcall(trunk);
    if not ok then
        hive.print(string.format("exec file: %s ... ... [failed]", filename));
        hive.print(err);
        return;
    end

    hive.print(string.format("load file: %s ... ... [ok]", filename));
end

local get_filenode = function(filename)
    local filenode = hive.files[filename];
    if filenode then
        return filenode;
    end

    local root_path = os.getenv("LUA_ROOT");
    local real_path = root_path and root_path..filename or filename;

    local env = {};
    setmetatable(env, hive.meta);
    filenode = {time=hive.get_file_time(real_path), env=env, path=real_path};
    hive.files[filename] = filenode; 
    return filenode;
end

hive.import = function(filename)
    local node = get_filenode(filename);
    local trunk, code_err = loadfile(node.path, "bt", node.env);
    if not trunk then
        error(code_err);
    end

    local ok, exec_err = pcall(trunk);
    if not ok then
        error(exec_err);
    end
end

function import(filename)
    local node = get_filenode(filename);
    try_load(filename, node);
    return node.env;
end

hive.reload = function()
    local now = os.time();
    for filename, filenode in pairs(hive.files) do
        local filetime = hive.get_file_time(filenode.path);
        if filetime ~= filenode.time and filetime ~= 0 and math.abs(now - filetime) > 1 then
            filenode.time = filetime;
            try_load(filename, filenode);
        end
    end
end
)__";


void hive_app::die(const std::string& err)
{
    std::string path = m_entry + ".err";
    FILE* file = fopen(path.c_str(), "w");
    if (file != nullptr)
    {
        fwrite(err.c_str(), err.length(), 1, file);
        fclose(file);
    }
    errx(1, "%s", err.c_str());
}

void hive_app::run(int argc, const char* argv[])
{
    lua_State* L = luaL_newstate();
    int64_t last_check = ::get_time_ms();
    const char* filename = argv[1];

    luaL_openlibs(L);
    m_entry = filename;
    lua_push_object(L, this);
    lua_push_object(L, this);
    lua_setglobal(L, "hive");
    lua_newtable(L);
    for (int i = 1; i < argc; i++)
    {
        lua_pushinteger(L, i - 1);
        lua_pushstring(L, argv[i]);
        lua_settable(L, -3);
    }
    lua_setfield(L, -2, "args");
    luaL_dostring(L, g_sandbox);

    std::string err;
    int top = lua_gettop(L);

    if(!lua_call_object_function(L, &err, this, "import", std::tie(), filename))
        die(err);

    while (lua_get_object_function(L, this, "run"))
    {
        if(!lua_call_function(L, &err, 0, 0))
            die(err);

        int64_t now = ::get_time_ms();
        if (now > last_check + m_reload_time)
        {
            lua_call_object_function(L, nullptr, this, "reload");
            last_check = now;
        }
        lua_settop(L, top);
    }

    lua_close(L);
}
