/*
** repository: https://github.com/trumanzhao/luna
** trumanzhao, 2017-05-13, trumanzhao@foxmail.com
*/

#include <stdio.h>
#include <locale>
#include "tools.h"
#include "hive.h"

int main(int argc, const char* argv[])
{
    tzset();
    setlocale(LC_ALL, "");

    if (argc < 2)
    {
        const char* fmt = R"--(hive %d.%d.%d https://github.com/trumanzhao/hive
usage: hive program_entry.lua ...
)--";
		printf(fmt, MAJOR_VERSION_NUMBER, MINOR_VERSION_NUMBER, REVISION_NUMBER);
        return 0;
    }

    g_app = new hive_app();
    g_app->run(argc, argv);
    delete g_app;
    return 0;
}
