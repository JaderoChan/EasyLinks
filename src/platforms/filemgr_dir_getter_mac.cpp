#include "filemgr_dir_getter.h"

#include <cstdio>   // FILE, popen, pclose, fgets
#include <stdexcept>
#include <string>

#include <qfileinfo.h>

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

QString getFocusedFileManagerDir()
{
    const char* script = R"(
        tell application "System Events"
            set frontApp to name of first application process where it is frontmost
        end tell

        if frontApp is not "Finder" then
            error "Front application is not Finder"
        end if

        tell application "Finder"
            if (count of Finder windows) = 0 then
                error "No Finder window is open"
            end if

            set targetFolder to (quoted form of POSIX path of (target of front window as alias))
            if targetFolder is missing value then
                error "Finder window has no valid target"
            end if

            return targetFolder
        end tell
    )";

    std::string cmd = "osascript -e '";
    cmd.append(script);
    cmd.append("'");
    std::string out = runCommand(cmd.c_str());

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
