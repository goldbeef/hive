/*
** repository: https://github.com/trumanzhao/luna
** trumanzhao, 2016/10/19, trumanzhao@foxmail.com
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <string>
#include <locale>
#include <cstdint>
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux
#include <unistd.h>
#include <dirent.h>
#endif
#ifdef __APPLE__
#include <unistd.h>
#include <mach/clock.h>
#include <mach/mach.h>
#endif
#include <list>
#include "tools.h"

time_t get_file_time(const char* file_name) {
    if (file_name == nullptr)
        return 0;

    struct stat file_info;
    int ret = stat(file_name, &file_info);
    if (ret != 0)
        return 0;

#ifdef __APPLE__
    return file_info.st_mtimespec.tv_sec;
#endif

#if defined(_WIN32) || defined(__linux)
    return file_info.st_mtime;
#endif
}

char* get_error_string(char buffer[], int len, int no) {
    buffer[0] = '\0';

#ifdef _WIN32
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, no, 0, buffer, len, nullptr);
#endif

#if defined(__linux) || defined(__APPLE__)
    strerror_r(no, buffer, len);
#endif

    return buffer;
}

void get_error_string(std::string& err, int no) {
    char txt[MAX_ERROR_TXT];
    get_error_string(txt, sizeof(txt), no);
    err = txt;
}

//将path拆成节点
/*
 * /abc/def/hig  ==> abc, def, hig
 * */
void path_to_nodes(std::list<std::string>& nodes, const char* path) {
       const char* token = path;
       const char* pos = path;
       while (true) {
                if (*pos == '\\' || *pos == '/' || *pos == '\0') {
                    if (pos > token) {
                        nodes.push_back(std::string(token, pos));
                    }
                    token = pos + 1;
                }
                if (*pos++ == '\0')
                    break;
       }
}

static bool is_abspath_path(const char* path) {
#ifdef _WIN32
    int drive = tolower(path[0]);
    return drive >= 'a' && drive <= 'z' && path[1] == ':';
#else
    return path[0] == '/';
#endif
}

static const int MAX_PATH_SIZE = 1024;

//根据path， 计算fullpath
bool get_full_path(std::string& fullpath, const char* path) {
    std::list<std::string> nodes;

    if (!is_abspath_path(path)) {
        //相对路径
        char cwd[MAX_PATH_SIZE];
#ifdef _WIN32
        if (!_getcwd(cwd, sizeof(cwd)))
            return false;
#else
        //获取工作目录， 解析出node
        if (!getcwd(cwd, sizeof(cwd)))
            return false;
#endif
        //path -> node
        path_to_nodes(nodes, cwd);
    }

    //绝对路径 或者 相对 路径的后半段
    path_to_nodes(nodes, path);

    std::string dot = ".";
    std::string dot2 = "..";
    std::list<std::string> tokens;
    for (auto& node : nodes) {
        if (node == dot2) {
            if (tokens.empty())
                return false;
            tokens.pop_back();
            continue;
        }

        if (node != dot) {
            tokens.push_back(node);
        }
    }

    fullpath.reserve(MAX_PATH_SIZE);
    for (auto& token : tokens) {
#ifdef _WIN32
        if (!fullpath.empty()) {
            fullpath.append("\\");
        }
#else
        fullpath.append("/");
#endif
        fullpath.append(token);
    }
    return true;
}



