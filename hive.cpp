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
#include "hive.h"
#include "tools.h"

hive_app* g_app = nullptr;

static void on_signal(int signo) {
    if (g_app) {
        g_app->set_signal(signo);
    }
}

LUA_EXPORT_CLASS_BEGIN(hive_app)
LUA_EXPORT_METHOD(get_version)
LUA_EXPORT_METHOD(get_file_time)
LUA_EXPORT_METHOD(get_full_path)
LUA_EXPORT_METHOD(get_time_ms)
LUA_EXPORT_METHOD(get_time_ns)
LUA_EXPORT_METHOD(sleep_ms)
LUA_EXPORT_METHOD(daemon)
LUA_EXPORT_METHOD(register_signal)
LUA_EXPORT_METHOD(default_signal)
LUA_EXPORT_METHOD(ignore_signal)
LUA_EXPORT_PROPERTY(m_signal)
LUA_EXPORT_PROPERTY(m_reload_time)
LUA_EXPORT_PROPERTY_READONLY(m_entry)
LUA_EXPORT_CLASS_END()

int hive_app::get_version(lua_State* L) {
    lua_pushinteger(L, MAJOR_VERSION_NUMBER);
    lua_pushinteger(L, MINOR_VERSION_NUMBER);
    lua_pushinteger(L, REVISION_NUMBER);
    return 3;
}

time_t hive_app::get_file_time(const char* filename) {
    return ::get_file_time(filename);
}

//查询绝对路径
int hive_app::get_full_path(lua_State* L) {
    const char* path = lua_tostring(L, 1);
    std::string fullpath;
    if (path != nullptr && ::get_full_path(fullpath, path)) {
        lua_pushstring(L, fullpath.c_str());
    }
    return 1;
}

int64_t hive_app::get_time_ms() {
    return ::get_time_ms();
}

int64_t hive_app::get_time_ns() {
    return ::get_time_ns();
}

void hive_app::sleep_ms(int ms) {
    ::sleep_ms(ms);
}

#ifdef _MSC_VER
void hive_app::daemon() { }
#endif

#if defined(__linux) || defined(__APPLE__)
void hive_app::daemon() {
    pid_t pid = fork();
    if (pid != 0)
        exit(0);

    setsid();
    umask(0);
    
    int null = open("/dev/null", O_RDWR); 
    if (null != -1) {
        dup2(null, STDIN_FILENO);
        dup2(null, STDOUT_FILENO);
        dup2(null, STDERR_FILENO);
        close(null);
    }
}
#endif

void hive_app::register_signal(int n) {
    signal(n, on_signal);
}

void hive_app::default_signal(int n) {
    signal(n, SIG_DFL);
}

void hive_app::ignore_signal(int n) {
    signal(n, SIG_IGN);
}

void hive_app::set_signal(int n) {
    uint64_t mask = 1;
    mask <<= n;
    m_signal |= mask;
}

static const char* g_sandbox = u8R"__(
hive.files = {};
hive.meta = {__index=function(t, k) return _G[k]; end}; --hive加载文件的env
hive.print = function(...) end; --do nothing

--加载单个节点的方法
local try_load = function(node)
    local fullpath = node.fullpath;
    local trunk, msg = loadfile(fullpath, "bt", node.env);
    if not trunk then
        hive.print(string.format("load file: %s ... ... [failed]", node.filename));
        hive.print(msg);
        return;
    end

    local ok, err = pcall(trunk);
    if not ok then
        hive.print(string.format("exec file: %s ... ... [failed]", node.filename));
        hive.print(err);
        return;
    end

    hive.print(string.format("load file: %s ... ... [ok]", node.filename));
end

--查询节点 文件名称-> node
local get_filenode = function(filename)
    local rootpath = os.getenv("LUA_ROOT");
    --todo
    local withroot = rootpath and hive.get_full_path(rootpath).."/"..filename or filename;
    local fullpath = hive.get_full_path(withroot) or withroot;

    --查询node
    local node = hive.files[fullpath];
    if node then
        return node;
    end

    local env = {};
    setmetatable(env, hive.meta);

    --给每个文件设置沙箱
    node = {env=env, fullpath=fullpath, filename=filename};
    hive.files[fullpath] = node;
    return node;
end

--加载lua主业务逻辑代码
hive.import = function(filename)
    --获取mtime
    local node = get_filenode(filename);
    if node.time then
        --已经加载过了
        return node.env;
    end

    --新加入的文件
    node.time = hive.get_file_time(node.fullpath);

    --load
    local trunk, code_err = loadfile(node.fullpath, "bt", node.env);
    if not trunk then
        error(code_err);
    end

    --call
    local ok, exec_err = pcall(trunk);
    if not ok then
        error(exec_err);
    end

    return node.env;
end

--lua文件中进行import文件，会尝试加载
function import(filename)
    local node = get_filenode(filename);
    if not node.time then
        node.time = hive.get_file_time(node.fullpath);
        try_load(node);
    end
    return node.env;
end

--reload: 重新加载所有的文件， 直到所有需要加载的文件被加载完成
hive.reload = function()
    local now = os.time();
    local update = true;
    while update do 
        update = false; --标记至少有一个文件更新成功
        for path, node in pairs(hive.files) do
            --遍历每一个文件
            local filetime = hive.get_file_time(node.fullpath);
            if filetime ~= node.time and filetime ~= 0 and math.abs(now - filetime) > 1 then
                --[[
                获取mitme成功; mtime 和 node的time发生变化; 时间戳差值 >1s;
                --]]
                node.time = filetime;
                update = true;
                try_load(node);
            end
        end
    end
    --如果没有一个文件更新成功则退出循环
end
)__";

void hive_app::die(const std::string& err) {
    std::string path = m_entry + ".err";
    FILE* file = fopen(path.c_str(), "w");
    if (file != nullptr) {
        fwrite(err.c_str(), err.length(), 1, file);
        fclose(file);
    }
    fprintf(stderr,"%s", err.c_str());
    exit(1);
}

//c++主运行逻辑, 该函数没有被导出
void hive_app::run(int argc, const char* argv[]) {
    lua_State* L = luaL_newstate();
    int64_t last_check = ::get_time_ms();
    const char* filename = argv[1];

    luaL_openlibs(L);
    m_entry = filename;
    // LUA_REGISTRYINDEX.__objects__.obj
    lua_push_object(L, this);

    // LUA_REGISTRYINDEX.__objects__.obj, LUA_REGISTRYINDEX.__objects__.obj
    lua_push_object(L, this);

    // LUA_REGISTRYINDEX.__objects__.obj
    // hive = LUA_REGISTRYINDEX.__objects__.obj
    lua_setglobal(L, "hive");

    // LUA_REGISTRYINDEX.__objects__.obj, t
    // hive = LUA_REGISTRYINDEX.__objects__.obj
    lua_newtable(L);
    for (int i = 1; i < argc; i++) {
        // LUA_REGISTRYINDEX.__objects__.obj, t, argIdx
        lua_pushinteger(L, i - 1);
        // LUA_REGISTRYINDEX.__objects__.obj, t, argIdx1, arg1
        lua_pushstring(L, argv[i]);

        // LUA_REGISTRYINDEX.__objects__.obj, t
        // t.argIdx1 = arg1
        lua_settable(L, -3);
    }
    // LUA_REGISTRYINDEX.__objects__.obj, t
    // hive = LUA_REGISTRYINDEX.__objects__.obj
    /**
     * t = {argIdx1 = arg1, argIdx2 = arg2 ...}
     *
     */

    // LUA_REGISTRYINDEX.__objects__.obj
    /*
     * hive = LUA_REGISTRYINDEX.__objects__.obj
     * LUA_REGISTRYINDEX.__objects__.obj.args = t
     * t = {argIdx1 = arg1, argIdx2 = arg2 ...}
     */
    lua_setfield(L, -2, "args");

    //执行沙箱代码
    luaL_dostring(L, g_sandbox);

    std::string err;
    // LUA_REGISTRYINDEX.__objects__.obj(tbl)
    /*
     * hive = LUA_REGISTRYINDEX.__objects__.obj
     * LUA_REGISTRYINDEX.__objects__.obj.args = t
     * t = {argIdx1 = arg1, argIdx2 = arg2 ...}
     */
    int top = lua_gettop(L);

    //记载主lua代码逻辑
    if(!lua_call_object_function(L, &err, this, "import", std::tie(), filename))
        die(err);

    //hive.run函数压栈
    while (lua_get_object_function(L, this, "run")) {
        if(!lua_call_function(L, &err, 0, 0))
            die(err);

        int64_t now = ::get_time_ms();
        if (m_reload_time > 0 && now > last_check + m_reload_time) {
            //尝试重新reload
            lua_call_object_function(L, nullptr, this, "reload");
            last_check = now;
        }
        //重置堆栈
        lua_settop(L, top);
    }

    lua_close(L);
}
