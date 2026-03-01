#include "worker.h"

#include <filesystem>
#include <mutex>
#include <stdexcept>

#include <qdir.h>

#include "rename_pattern.h"

#define THROW_RTERR(errorMsg) throw std::runtime_error(errorMsg)

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

constexpr int ProgressUpdateInterval = 20;

FileLinkWorker::FileLinkWorker(QObject* parent)
    : QObject(parent)
{}

FileLinkWorker::~FileLinkWorker()
{
    cancel();
}

bool FileLinkWorker::isWindowsSymlink(const QFileInfo& fileinfo)
{
    return fileinfo.isShortcut() || fileinfo.isJunction();
}

void FileLinkWorker::createLink(LinkType linkType, const QFileInfo& source, const QFileInfo& target)
{
    namespace fs = std::filesystem;

    // 尝试创建目标文件的父路径。
    QDir targetDir(target.absolutePath());
    if (!targetDir.exists())
    {
        if (!targetDir.mkpath("."))
            THROW_RTERR("The target path cannot be created");
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
                THROW_RTERR("The source file is of an unsupported entity type");
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

void FileLinkWorker::run()
{
    progressUpdateTimer_.start();

    // 收集条目并创建链接任务。
    collectEntries();
    // 强制更新当前进度。
    tryUpdateProgress(true);

    // 处理用户是否已暂停或取消操作。
    if (processPauseAndCancel())
    {
        emit finished();
        return;
    }

    // 处理链接任务。
    tasks_ = processTasks();
    tryUpdateProgress(true);

    if (processPauseAndCancel())
    {
        emit finished();
        return;
    }

    // 如果冲突项列表为空则直接报告操作已完成并返回。
    if (tasks_.isEmpty())
    {
        emit finished();
        return;
    }

    // 因冲突项列表非空，等待用户决定。
    waitConflictsDecision();
    if (processPauseAndCancel())
    {
        emit finished();
        return;
    }

    // 如果用户使用Skip策略并应用至所有冲突项，则只更新统计数据。
    if (cesApplyToAll_ && cesOfAll_ == CES_SKIP)
        stats_.processedEntries += stats_.conflicts;
    else
        processTasks();

    tryUpdateProgress(true);
    emit finished();
}

void FileLinkWorker::setParameters(
    LinkType linkType,
    const QStringList& sourcePaths,
    const QString& targetDir,
    const QString& renamePattern,
    bool removeToTrash)
{
    linkType_ = linkType;
    sourcePaths_ = sourcePaths;
    targetDir_ = targetDir;
    renamePattern_ = renamePattern;
    if (!isLegalRenamePattern(renamePattern_))
        renamePattern_ = DEFAULT_RENAME_PATTERN;
    removeToTrash_ = removeToTrash;
}

void FileLinkWorker::setConflictsDecisionForAll(ConflictingEntryStrategy ces)
{
    cesOfAll_ = ces;
    cesApplyToAll_ = true;
    resume();
}

void FileLinkWorker::setConflictsDecision(const LinkTasks& tasks)
{
    tasks_ = tasks;
    resume();
}

void FileLinkWorker::pause()
{
    paused_ = true;
}

void FileLinkWorker::resume()
{
    paused_ = false;
    pausedCondition_.notify_all();
}

void FileLinkWorker::cancel()
{
    cancelled_ = true;
    paused_ = false;
    pausedCondition_.notify_all();
}

bool FileLinkWorker::processPauseAndCancel()
{
    if (cancelled_)
        return true;

    std::mutex dummyMutex;
    std::unique_lock<std::mutex> locker(dummyMutex);
    pausedCondition_.wait(locker, [=]() { return !paused_.load(); });

    return cancelled_;
}

void FileLinkWorker::addTask(LinkType linkType, const QFileInfo& source, const QFileInfo& target)
{
    currentEntryPair_ = {{source}, {target}};
    tasks_.enqueue({linkType, CES_NONE, currentEntryPair_});
    stats_.totalEntries++;
    tryUpdateProgress();
}

bool FileLinkWorker::createLink(LinkType linkType, QFileInfo& source, QFileInfo& target, ConflictingEntryStrategy ces)
{
    source.refresh();
    target.refresh();

    if (source.exists())
    {
        if (!target.exists())
        {
            createLink(linkType, source, target);
        }
        else if (target.isFile() || isWindowsSymlink(target))
        {
            ces = cesApplyToAll_ ? cesOfAll_.load() : ces;
            switch (ces)
            {
                case CES_NONE:
                    return false;
                case CES_REPLACE:
                {
                    if (source.absoluteFilePath() == target.absoluteFilePath())
                    {
                        THROW_RTERR("The replaced file and the replacement file are the same entity");
                    }

                    QString targetOriginName = target.absoluteFilePath();
                    if (removeToTrash_)
                    {
                        QString filepathInTrash;
                        if (!QFile::moveToTrash(targetOriginName, &filepathInTrash))
                            THROW_RTERR("Failed to move the target file to trash");
                        try
                        {
                            createLink(linkType, source, target);
                        }
                        catch(const std::exception& e)
                        {
                            QFile::rename(filepathInTrash, targetOriginName);
                            THROW_RTERR(e.what());
                        }
                    }
                    else
                    {
                        QString targetTempName = target.absoluteDir().filePath(target.fileName() + APP_UUID);
                        if (!QFile::rename(targetOriginName, targetTempName))
                            THROW_RTERR("Failed to rename the target file");
                        try
                        {
                            createLink(linkType, source, target);
                            QFile::remove(targetTempName);
                        }
                        catch(const std::exception& e)
                        {
                            QFile::rename(targetTempName, targetOriginName);
                            THROW_RTERR(e.what());
                        }
                    }
                    break;
                }
                case CES_SKIP:
                    break;
                case CES_KEEP:
                    generateNewPath(target);
                    createLink(linkType, source, target);
                    break;
                default:
                    break;
            }
        }
        else
        {
            THROW_RTERR("The target directory exists nonregular file entities of the same name");
        }
    }
    else
    {
        THROW_RTERR("The source file does not exist");
    }

    return true;
}

LinkTasks FileLinkWorker::processTasks()
{
    LinkTasks conflicts;
    while (!tasks_.isEmpty())
    {
        if (processPauseAndCancel())
            return conflicts;

        auto task = tasks_.dequeue();
        currentEntryPair_ = task.entryPair;
        try
        {
            auto& source = task.entryPair.source.fileinfo;
            auto& target = task.entryPair.target.fileinfo;
            bool isConflict = !createLink(task.linkType, source, target, task.ces);
            if (isConflict)
            {
                stats_.conflicts++;
                currentEntryPair_.source.updatePreInfo();
                currentEntryPair_.target.updatePreInfo();
                conflicts.append(task);
            }
            else
            {
                stats_.processedEntries++;
            }

            tryUpdateProgress();
        }
        catch (const std::exception& e)
        {
            stats_.processedEntries++;
            stats_.failedEntries++;
            emit errorOccurred(task.linkType, currentEntryPair_, QString::fromLocal8Bit(e.what()));
            tryUpdateProgress();
        }
    }
    return conflicts;
}

void FileLinkWorker::tryUpdateProgress(bool forced)
{
    if (forced)
    {
        emit progressUpdated(currentEntryPair_, stats_);
    }
    else if (progressUpdateTimer_.elapsed() > ProgressUpdateInterval)
    {
        progressUpdateTimer_.restart();
        emit progressUpdated(currentEntryPair_, stats_);
    }
}

void FileLinkWorker::waitConflictsDecision()
{
    emit conflictsDecisionWaited(tasks_);
    pause();
}

void FileLinkWorker::collectEntriesForSymlink()
{
    for (const auto& sourcePath : sourcePaths_)
    {
        if (processPauseAndCancel())
            return;
        QFileInfo source(sourcePath);
        QFileInfo target(targetDir_ + "/" + source.fileName());
        addTask(LT_SYMLINK, source, target);
    }
}

void FileLinkWorker::collectEntriesForHardlinkHelper(const QString& sourcePath, const QString& targetDir)
{
    if (processPauseAndCancel())
        return;

    QFileInfo source(sourcePath);
    if (source.isFile() || isWindowsSymlink(source))
    {
        QFileInfo target(targetDir + "/" + source.fileName());
        addTask(LT_HARDLINK, source, target);
    }
    else if (source.isDir())
    {
        QDir sourceDir(sourcePath);
        QFileInfoList entries = sourceDir.entryInfoList(
            QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System
        );

        for (auto& entry : entries)
        {
            if (processPauseAndCancel())
                return;

            QFileInfo target(targetDir + "/" + entry.fileName());
            if (entry.isFile() || isWindowsSymlink(entry))
                addTask(LT_HARDLINK, entry, target);
            else
                collectEntriesForHardlinkHelper(entry.absoluteFilePath(), target.absoluteFilePath());
        }
    }
}

void FileLinkWorker::collectEntriesForHardlink()
{
    for (const auto& sourcePath : sourcePaths_)
    {
        if (processPauseAndCancel())
            return;

        QFileInfo source(sourcePath);
        if (source.isDir())
            collectEntriesForHardlinkHelper(sourcePath, targetDir_ + "/" + source.fileName());
        else
            collectEntriesForHardlinkHelper(sourcePath, targetDir_);
    }
}

void FileLinkWorker::collectEntries()
{
    switch (linkType_)
    {
        case LT_SYMLINK:
            collectEntriesForSymlink();
            break;
        case LT_HARDLINK:
            collectEntriesForHardlink();
            break;
        default:
            break;
    }
}

void FileLinkWorker::generateNewPath(QFileInfo& file)
{
    auto baseName = file.completeBaseName();
    auto suffix = file.suffix();
    auto path = file.path();
    int counter = 1;

    do
    {
        QString newPath = path + "/" + parseRenamePattern(renamePattern_, baseName, counter++) + "." + suffix;
        file.setFile(newPath);
    } while (file.exists());
}
