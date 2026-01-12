#include "worker.h"

#include <filesystem>
#include <mutex>
#include <stdexcept>

#include <qdir.h>

#define THROW_RTERR(errorMsg) throw std::runtime_error(errorMsg)

constexpr int ProgressUpdateInterval = 20;

// todo: 优化对符号链接条目的支持（消歧义等）

FileLinkWorker::FileLinkWorker(QObject* parent)
    : QObject(parent)
{}

FileLinkWorker::~FileLinkWorker()
{
    qDebug() << "~FileLinkWorker";
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

void FileLinkWorker::createLink(LinkType linkType, QFileInfo source, QFileInfo target)
{
    namespace fs = std::filesystem;

    QDir targetDir(target.absolutePath());
    if (!targetDir.exists())
    {
        if (!targetDir.mkpath("."))
            THROW_RTERR("The target path cannot be created.");
    }

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

    collectEntries();
    tryUpdateProgress(true);

    qDebug() << "Collect finished";
    if (processPauseAndCancel())
    {
        emit finished();
        return;
    }

    tasks_ = processTasks();
    qDebug() << "Process finished 1";
    tryUpdateProgress(true);

    if (processPauseAndCancel())
    {
        emit finished();
        return;
    }

    if (tasks_.isEmpty())
    {
        emit finished();
        return;
    }

    qDebug() << "Wait conflict decision";
    waitConflictsDecision();
    if (processPauseAndCancel())
    {
        emit finished();
        return;
    }

    processTasks();
    qDebug() << "Process finished 2";
    tryUpdateProgress(true);
    emit finished();
}

void FileLinkWorker::setParameters(
    LinkType linkType,
    const QStringList& sourcePaths,
    const QString& targetDir)
{
    linkType_ = linkType;
    sourcePaths_ = sourcePaths;
    targetDir_ = targetDir;
}

void FileLinkWorker::setConflictsDecisionForAll(EntryConflictStrategy ecs)
{
    ECSOfAll = ecs;
    ECSApplyToAll_ = true;
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

void FileLinkWorker::addTask(LinkType linkType, QFileInfo sourceEntry, QFileInfo targetEntry)
{
    currentEntryPair_ = {sourceEntry, targetEntry};
    tasks_.enqueue({linkType, currentEntryPair_, ECS_NONE});
    stats_.totalEntries++;
    tryUpdateProgress();
}

bool FileLinkWorker::createLink(
    LinkType linkType,
    QFileInfo source,
    QFileInfo target,
    EntryConflictStrategy ecs)
{
    source.refresh();
    target.refresh();

    if (!isOnSameDriver(source.absoluteFilePath(), target.absoluteFilePath()))
        THROW_RTERR("The specified original file and the target are "
                    "located on different devices or file systems.");

    if (source.exists())
    {
        if (!target.exists())
        {
            createLink(linkType, source, target);
        }
        else if (target.isFile())
        {
            ecs = ECSApplyToAll_ ? ECSOfAll.load() : ecs;
            switch (ecs)
            {
                case ECS_NONE:
                    return true;
                case ECS_REPLACE:
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
                case ECS_SKIP:
                    break;
                case ECS_KEEP:
                    target = generateNewPath(target);
                    createLink(linkType, source, target);
                    break;
                default:
                    break;
            }
        }
        else
        {
            THROW_RTERR("There are nonregular file entities with the "
                        "same name in the target directory.");
        }
    }
    else
    {
        THROW_RTERR("The source file does not exist.");
    }

    return false;
}

LinkTasks FileLinkWorker::processTasks()
{
    LinkTasks conflictTasks;
    while (!tasks_.isEmpty())
    {
        if (processPauseAndCancel())
            return conflictTasks;

        auto task = tasks_.dequeue();
        currentEntryPair_ = task.entryPair;
        try
        {
            bool isConflict = createLink(
                task.linkType,
                task.entryPair.source,
                task.entryPair.target,
                task.ecs
            );

            if (isConflict)
            {
                stats_.conflicts++;
                conflictTasks.append(task);
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
    return conflictTasks;
}

void FileLinkWorker::tryUpdateProgress(bool forced)
{
    if (forced)
    {
        emit progressUpdated(currentEntryPair_, stats_);
        return;
    }

    if (progressUpdateTimer_.elapsed() > ProgressUpdateInterval)
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
        QFileInfo sourceEntry(sourcePath);
        QFileInfo targetEntry(targetDir_ + "/" + sourceEntry.fileName());
        addTask(LT_SYMLINK, sourceEntry, targetEntry);
    }
}

void FileLinkWorker::collectEntriesForHardlinkHelper(
    const QString& sourcePath,
    const QString& targetDir)
{
    if (processPauseAndCancel())
        return;

    if (QFileInfo(sourcePath).isDir())
    {
        QDir sourceDir(sourcePath);
        QFileInfoList sourceEntries = sourceDir.entryInfoList(
            QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System
        );

        for (const auto& sourceEntry : sourceEntries)
        {
            if (processPauseAndCancel())
                return;

            QFileInfo targetEntry(targetDir + "/" + sourceEntry.fileName());
            if (sourceEntry.isDir())
            {
                collectEntriesForHardlinkHelper(
                    sourceEntry.absoluteFilePath(),
                    targetEntry.absoluteFilePath());
            }
            else
            {
                addTask(LT_HARDLINK, sourceEntry, targetEntry);
            }
        }
    }
    else
    {
        QFileInfo sourceEntry(sourcePath);
        QFileInfo targetEntry(targetDir + "/" + sourceEntry.fileName());
        addTask(LT_HARDLINK, sourceEntry, targetEntry);
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

// todo: 支持用户自定义重命名规则。
QFileInfo FileLinkWorker::generateNewPath(QFileInfo file)
{
    auto baseName = file.completeBaseName();
    auto suffix = file.suffix();
    auto path = file.path();
    int counter = 1;

    do
    {
        file.setFile(
            QString("%1/%2 (%3).%4").arg(path, baseName, QString::number(counter++), suffix)
        );
    } while (file.exists());

    return file;
}
