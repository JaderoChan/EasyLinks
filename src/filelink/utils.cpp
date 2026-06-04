#include "utils.h"

#include <filesystem>
#include <stdexcept>

#include <qdir.h>

#ifdef Q_OS_WIN

#define LONG_PATH(p) (!fix::isNeeded(p) ? p : fix::apply(p))

namespace fix
{

static inline bool isUnc(const std::filesystem::path& p) noexcept
{
    auto pathStr = p.native();
    return pathStr.size() < 2 ? false : (pathStr[0] == L'\\' && pathStr[1] == L'\\');
}

static inline bool isNeeded(const std::filesystem::path& p) noexcept
{
    return p.is_absolute() && !isUnc(p);
}

static inline std::filesystem::path apply(const std::filesystem::path& p) noexcept
{
    return LR"(\\?\)" + p.lexically_normal().native();
}

} // namespace fix

#endif // Q_OS_WIN

bool isWindowsSymlink(const QFileInfo& fileinfo)
{
    return fileinfo.isShortcut() || fileinfo.isJunction();
}

void createLink(LinkType linkType, const QFileInfo& source, const QFileInfo& target)
{
    namespace fs = std::filesystem;

    // 尝试创建目标文件的父路径。
    QDir targetDir(target.absolutePath());
    if (!targetDir.exists())
    {
        if (!targetDir.mkpath("."))
            throw std::runtime_error("The target path cannot be created");
    }

#ifdef Q_OS_WIN
    auto sourcePath = LONG_PATH(source.filesystemAbsoluteFilePath());
    auto targetPath = LONG_PATH(target.filesystemAbsoluteFilePath());
#else
    auto sourcePath = source.filesystemAbsoluteFilePath();
    auto targetPath = target.filesystemAbsoluteFilePath();
#endif // Q_OS_WIN

    switch (linkType)
    {
        case LT_SYMLINK:
        {
            if (source.isFile())
                fs::create_symlink(sourcePath, targetPath);
            else if (source.isDir())
                fs::create_directory_symlink(sourcePath, targetPath);
            else
                throw std::runtime_error("The source file is of an unsupported entity type");
            break;
        }
        case LT_HARDLINK:
        {
            fs::create_hard_link(sourcePath, targetPath);
            break;
        }
        default:
        {
            break;
        }
    }
}
