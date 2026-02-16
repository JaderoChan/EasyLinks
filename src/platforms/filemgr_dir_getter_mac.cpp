#include "filemgr_dir_getter.h"

#include <cstdio>   // FILE, popen, pclose, fgets
#include <stdexcept>
#include <string>

#include <qfile.h>
#include <qfileinfo.h>

#include "config.h"

static std::string runCommand(const std::string& cmd)
{
    char buf[256] = {0};
    std::string out;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        throw std::runtime_error("Failed to open pipe");
    while (fgets(buf, sizeof(buf), pipe))
        out += buf;
    pclose(pipe);

    return out;
}

QString getFocusedFileManagerDir()
{
    static const std::string script =
        QFile(APP_RESOURCE_DIRPATH + "scripts/get_focused_finder_dir.sh").readAll().toStdString();
    static const std::string cmd = "osascript -e '" + script + "'";
    std::string out = runCommand(cmd);

    QString path = QString::fromStdString(out);
    // 去除首尾空白字符。
    path = path.trimmed();
    // 如果成功返回路径，其会被单引号包裹。
    if (path.size() < 2 || !path.startsWith('\'') || !path.endsWith('\''))
        throw std::runtime_error(std::string("Failed to get directory path from Finder"));
    // 去除首尾引号。
    path.remove(0, 1);
    path.chop(1);

    if (!QFileInfo(path).isAbsolute())
        throw std::runtime_error("Failed to get directory path from Finder");

    return path;
}
