﻿/*
** repository: https://github.com/trumanzhao/luna
** trumanzhao, 2017-05-13, trumanzhao@foxmail.com
*/

#pragma once

#include "luna.h"

 const int MAJOR_VERSION_NUMBER = 0;
 const int MINOR_VERSION_NUMBER = 1;
 const int REVISION_NUMBER = 1;

class hive_app final {
public:
    hive_app() { }
    ~hive_app() { }
    int get_version(lua_State* L);
    time_t get_file_time(const char* filename);
    int get_full_path(lua_State* L);
    int64_t get_time_ms();
    int64_t get_time_ns();
    void sleep_ms(int ms);
    void daemon();
    void register_signal(int n);
    void default_signal(int n);
    void ignore_signal(int n);
public:
    void __gc() { } // custom gc, just do nothing
    void set_signal(int n);
    void die(const std::string& err);
    void run(int argc, const char* argv[]);

    DECLARE_LUA_CLASS(hive_app);
private:
    uint64_t m_signal = 0;
    int m_reload_time = 3000;
    std::string m_entry;
};

extern hive_app* g_app;
