#include "filemgr_dir_getter.h"

#include <cstdio>   // FILE, popen, pclose, fgets
#include <stdexcept>
#include <string>

static std::string runCommand(const char* cmd)
{
    char buf[256] = {0};
    std::string out;

    FILE* pipe = popen(cmd, "r");
    if (!pipe)
        throw std::runtime_error("Failed to open pipe");
    while (fgets(buf, sizeof(buf), pipe))
        out += buf;
    pclose(pipe);

    return out;
}

QString getDirectoryOfFocusedFileManager()
{
    const char* cmd =
        R"(osascript -e 'tell application "Finder" to set thePath to (quoted form of POSIX path of (target of front window as alias))')";

    try
    {
        std::string out = runCommand(cmd);
        QString path = QString::fromStdString(out);
        // 去除首尾空白字符。
        path = path.trimmed();
        // 如果成功返回路径，其会被单引号包裹。
        if (path.size() < 2 || !path.startsWith('\'') || !path.endsWith('\''))
            throw std::runtime_error("Failed to get directory path from Finder");
        // 去除首尾引号
        path.remove(0, 1);
        path.chop(1);

        return path;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("Failed to run osascript: ") + e.what());
    }
}
