#include "worker.h"

#include <filesystem>
#include <mutex>
#include <stdexcept>

#include <qdir.h>

#define THROW_RTERR(errorMsg) throw std::runtime_error(errorMsg)

constexpr int ProgressUpdateInterval = 20;

FileLinkWorker::FileLinkWorker(QObject* parent)
    : QObject(parent)
{}

FileLinkWorker::~FileLinkWorker()
{
    cancel();
}

bool FileLinkWorker::isOnSameDriver(const QString& a, const QString& b)
{
#ifdef Q_OS_WIN
    QDir dirA(a);
    QDir dirB(b);
    if (!a.isEmpty() && !b.isEmpty())
        return dirA.rootPath().left(3).toUpper() == dirB.rootPath().left(3).toUpper();
    return false;
#else
    // todo
#endif
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
            THROW_RTERR("The target path cannot be created.");
    }

    // 如果原文件与目标文件是同一个文件实体则抛出异常。
    auto sourcePath = source.filesystemAbsoluteFilePath();
    auto targetPath = target.filesystemAbsoluteFilePath();
    if (fs::exists(sourcePath) && fs::exists(targetPath) && fs::equivalent(sourcePath, targetPath))
        THROW_RTERR("The source file and the target file are the same entity.");

    switch (linkType)
    {
        case LT_SYMLINK:
        {
            if (source.isFile())
                fs::create_symlink(sourcePath, targetPath);
            else if (source.isDir())
                fs::create_directory_symlink(sourcePath, targetPath);
            else
                THROW_RTERR("The source file is of an unsupported entity type.");
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
    if (cesApplyToAll_ && cesOfAll == CES_SKIP)
    {
        stats_.processedEntries += stats_.conflicts;
        stats_.successfulEntries += stats_.conflicts;
    }
    else
    {
        processTasks();
    }

    tryUpdateProgress(true);
    emit finished();
}

void FileLinkWorker::setParameters(
    LinkType linkType,
    const QStringList& sourcePaths,
    const QString& targetDir,
    bool removeToTrash)
{
    linkType_ = linkType;
    sourcePaths_ = sourcePaths;
    targetDir_ = targetDir;
    removeToTrash_ = removeToTrash;
}

void FileLinkWorker::setConflictsDecisionForAll(ConflictingEntryStrategy ces)
{
    cesOfAll = ces;
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
    tasks_.enqueue({linkType, currentEntryPair_, CES_NONE});
    stats_.totalEntries++;
    tryUpdateProgress();
}

bool FileLinkWorker::createLink(LinkType linkType, QFileInfo& source, QFileInfo& target, ConflictingEntryStrategy ces)
{
    source.refresh();
    target.refresh();

    // 如果创建的是硬链接且原文件与目标文件处于不同驱动器上则抛出错误。
    // （此情况在绝大多数时候一定会链接失败，为了防止目标文件已存在且用户决定替换目标文件而导致目标文件被无端删除）
    if (linkType == LT_HARDLINK && !isOnSameDriver(source.absoluteFilePath(), target.absoluteFilePath()))
        THROW_RTERR("The specified original file and the target are located on different devices or file systems.");

    if (source.exists())
    {
        if (!target.exists())
        {
            createLink(linkType, source, target);
        }
        else if (target.isFile() || isWindowsSymlink(target))
        {
            ces = cesApplyToAll_ ? cesOfAll.load() : ces;
            switch (ces)
            {
                case CES_NONE:
                    return false;
                case CES_REPLACE:
                    if (removeToTrash_)
                    {
                        if (!QFile::moveToTrash(target.absoluteFilePath()))
                            THROW_RTERR("Failed to move the target file to trash.");
                    }
                    else
                    {
                        if (!QFile::remove(target.absoluteFilePath()))
                            THROW_RTERR("Failed to remove the target file.");
                    }
                    createLink(linkType, source, target);
                    break;
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
            THROW_RTERR("There are nonregular file entities with the same name in the target directory.");
        }
    }
    else
    {
        THROW_RTERR("The source file does not exist.");
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
                stats_.successfulEntries++;
            }

            tryUpdateProgress();
        }
        catch (std::exception& e)
        {
            stats_.processedEntries++;
            stats_.failedEntries++;
            emit errorOccurred(task.linkType, currentEntryPair_, e.what());
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
        file.setFile(QString("%1/%2 (%3).%4").arg(path, baseName, QString::number(counter++), suffix));
    } while (file.exists());
}
