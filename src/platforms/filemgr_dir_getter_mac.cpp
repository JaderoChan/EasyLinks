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
    const char* script =
        "/usr/bin/osascript 2>&1 -e "
        "\"tell application \\\"System Events\\\" "
        "set frontApp to name of first application process whose frontmost is true "
        "end tell "
        "if frontApp is not \\\"Finder\\\" then error \\\"Front window is not Finder\\\" "
        "tell application \\\"Finder\\\" "
        "if (count of Finder windows) = 0 then error \\\"No Finder window is open\\\" "
        "set targetFolder to target of front Finder window "
        "if targetFolder is missing value then error \\\"No folder is selected\\\" "
        "return POSIX path of targetFolder "
        "end tell\"";

    try
    {
        std::string out = runCommand(script);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("Failed to run osascript: ") + e.what());
    }

    // 去除末尾换行符
    while (!out.empty() && (out.back() == '\n' || out.back() == '\r'))
        out.pop_back();

    if (out.empty())
        throw std::runtime_error("Failed to get directory path from Finder");

    return QString::fromStdString(out);
}
