#include "pattern_link_worker.h"

#include <mutex>
#include <stdexcept>
#include <algorithm>

#include <qdir.h>
#include <qfile.h>
#include <qhash.h>
#include <quuid.h>

#include <utils/file_io.h>
#include "utils.h"

#define THROW_RTERR(errorMsg) throw std::runtime_error(errorMsg)

constexpr unsigned int PROGRESS_UPDATE_INTERVAL_MS = 20;

static void removeSingleEntryGroups(QList<QFileInfoList>& entryGroups)
{
    for (int i = entryGroups.size() - 1; i >= 0; --i)
    {
        if (entryGroups[i].size() < 2)
            entryGroups.removeAt(i);
    }
}

static int computeTotalLinkTasks(const QList<QFileInfoList>& entryGroups)
{
    int total = 0;
    for (const auto& group : entryGroups)
    {
        if (group.size() > 1)
            total += (group.size() - 1);
    }
    return total;
}

static QString makeEntryKey(const QFileInfo& entry)
{
    return QDir::cleanPath(entry.absoluteFilePath());
}

static bool isSubPathOf(const QString& childPath, const QString& parentPath)
{
    if (childPath == parentPath)
        return true;

    QString parentPrefix = parentPath;
    if (!parentPrefix.endsWith('/'))
        parentPrefix += '/';
    return childPath.startsWith(parentPrefix);
}

static QStringList reduceRootDirs(const QStringList& dirList)
{
    QStringList normalized;
    normalized.reserve(dirList.size());
    for (const auto& dir : dirList)
    {
        if (dir.isEmpty())
            continue;

        QFileInfo fi(dir);
        QString normalizedPath = fi.canonicalFilePath();
        if (normalizedPath.isEmpty())
            normalizedPath = fi.absoluteFilePath();
        normalizedPath = QDir::cleanPath(normalizedPath);

        if (!normalizedPath.isEmpty() && !normalized.contains(normalizedPath))
            normalized.append(normalizedPath);
    }

    std::sort(normalized.begin(), normalized.end(), [](const QString& a, const QString& b)
    {
        if (a.size() != b.size())
            return a.size() < b.size();
        return a < b;
    });

    QStringList reduced;
    for (const auto& path : normalized)
    {
        bool covered = false;
        for (const auto& kept : reduced)
        {
            if (isSubPathOf(path, kept))
            {
                covered = true;
                break;
            }
        }

        if (!covered)
            reduced.append(path);
    }

    return reduced;
}

PatternLinkWorker::PatternLinkWorker(QObject* parent)
    : QObject(parent)
{}

PatternLinkWorker::~PatternLinkWorker()
{
    cancel();
}

void PatternLinkWorker::setParameters(
    const QStringList& dirList, Patterns patterns, bool needReview, bool removeToTrash)
{
    dirs_       = reduceRootDirs(dirList);
    patterns_   = patterns;
    needReview_ = needReview;
    removeToTrash_ = removeToTrash;
}

void PatternLinkWorker::run()
{
    progressUpdateTimer_.restart();
    stats_ = LinkStats();
    entryGroups_.clear();
    entryIdToGroupIdxMap_.clear();
    scannedEntryKeys_.clear();

    collectEntryGroups();
    removeSingleEntryGroups(entryGroups_);
    stats_.totalEntries = computeTotalLinkTasks(entryGroups_);
    tryUpdateProgress(true);

    // If canceled during collection, stop before review/work.
    if (cancelled_)
    {
        entryGroups_.clear();
        stats_ = LinkStats();
        tryUpdateProgress(true);
        emit finished();
        return;
    }

    if (needReview_ && !entryGroups_.isEmpty())
    {
        emit reviewRequested(&entryGroups_);
        pause();

        if (processPauseAndCancel())
        {
            emit finished();
            return;
        }

        removeSingleEntryGroups(entryGroups_);
        stats_.totalEntries = computeTotalLinkTasks(entryGroups_);
        tryUpdateProgress(true);
    }

    work();
    tryUpdateProgress(true);
    emit finished();
}

void PatternLinkWorker::pause()
{
    std::lock_guard<std::mutex> locker(pausedMtx_);
    paused_ = true;
}

void PatternLinkWorker::resume()
{
    {
        std::lock_guard<std::mutex> locker(pausedMtx_);
        paused_ = false;
    }
    pausedCond_.notify_one();
}

void PatternLinkWorker::cancel()
{
    cancelled_ = true;
    {
        std::lock_guard<std::mutex> locker(pausedMtx_);
        paused_ = false;
    }
    pausedCond_.notify_one();
}

void PatternLinkWorker::finishReview()
{
    resume();
}

bool PatternLinkWorker::processPauseAndCancel()
{
    if (cancelled_)
        return true;

    std::unique_lock<std::mutex> locker(pausedMtx_);
    pausedCond_.wait(locker, [=]() { return !paused_; });

    return cancelled_;
}

void PatternLinkWorker::tryUpdateProgress(bool forced)
{
    if (forced)
    {
        emit progressUpdated(currentFiPair_, stats_);
    }
    else if (progressUpdateTimer_.elapsed() > PROGRESS_UPDATE_INTERVAL_MS)
    {
        progressUpdateTimer_.restart();
        emit progressUpdated(currentFiPair_, stats_);
    }
}

void PatternLinkWorker::work()
{
    for (auto& entrys : entryGroups_)
    {
        if (entrys.empty())
            continue;

        currentFiPair_.master = entrys.front();
        entrys.pop_front();
        while (!entrys.empty())
        {
            if (processPauseAndCancel())
                return;

            currentFiPair_.slave = entrys.front();
            entrys.pop_front();
            stats_.processedEntries++;

            try
            {
                currentFiPair_.slave.refresh();
                if (currentFiPair_.slave.isFile() || isWindowsSymlink(currentFiPair_.slave))
                {
                    const QString targetPath = currentFiPair_.slave.filePath();

                    if (removeToTrash_)
                    {
                        QString pathInTrash;
                        if (!QFile::moveToTrash(targetPath, &pathInTrash))
                            THROW_RTERR("Failed to move the target file to trash.");

                        try
                        {
                            createLink(LT_HARDLINK, currentFiPair_.master, currentFiPair_.slave);
                        }
                        catch (const std::exception&)
                        {
                            QFile::rename(pathInTrash, targetPath);
                            throw;
                        }
                    }
                    else
                    {
                        const QString backupPath = targetPath + ".easylinks.bak." +
                            QUuid::createUuid().toString(QUuid::WithoutBraces);

                        if (!QFile::rename(targetPath, backupPath))
                            THROW_RTERR("Failed to move the target file to backup path.");

                        try
                        {
                            createLink(LT_HARDLINK, currentFiPair_.master, currentFiPair_.slave);
                            QFile::remove(backupPath);
                        }
                        catch (const std::exception&)
                        {
                            QFile::rename(backupPath, targetPath);
                            throw;
                        }
                    }
                }
                else
                {
                    THROW_RTERR("The target file does nt exist or it's a directory.");
                }
            }
            catch (std::exception& e)
            {
                stats_.failedEntries++;
                emit errorOccurred(currentFiPair_, e.what());
            }

            tryUpdateProgress();
        }
    }
}

static inline uint64_t fmix64(uint64_t x)
{
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}

static uint64_t computeFileHash(const QString& filepath)
{
    const auto bytes = readFileContent<false>(filepath);
    return qHash(bytes);
}

static uint64_t makeEntryId(const QFileInfo& entry, Patterns patterns)
{
    uint64_t hashes[4] = {0};
    hashes[0] = (patterns & PATTERN_SAME_NAME) ? qHash(entry.fileName())           : 0;
    hashes[1] = (patterns & PATTERN_SAME_SIZE) ? qHash(entry.size())               : 0;
    hashes[2] = (patterns & PATTERN_SAME_PERM) ? qHash(entry.permissions())        : 0;
    hashes[3] = (patterns & PATTERN_SAME_HASH) ? computeFileHash(entry.filePath()) : 0;

    uint64_t acc = 0x9e3779b97f4a7c15ULL;
    for (uint64_t x : hashes)
        acc += fmix64(x);
    acc ^= static_cast<uint64_t>(sizeof(hashes) / sizeof(uint64_t)) * 0x9e3779b97f4a7c15ULL;
    return fmix64(acc);
}

void PatternLinkWorker::collectEntryGroups(const QString& dir)
{
    QDir qdir(dir);
    const QDir::Filters filters(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    QFileInfoList enteries = qdir.entryInfoList(filters);

    for (const auto& entry : enteries)
    {
        if (processPauseAndCancel())
            return;

        if (entry.isDir())
        {
            collectEntryGroups(entry.filePath());
            continue;
        }

        if (!(entry.isFile() || isWindowsSymlink(entry)))
            continue;

        const QString entryKey = makeEntryKey(entry);
        if (scannedEntryKeys_.contains(entryKey))
            continue;
        scannedEntryKeys_.insert(entryKey);

        currentFiPair_ = {entry, entry};
        uint64_t id = 0;
        try
        {
            id = makeEntryId(entry, patterns_);
        }
        catch (const std::exception& e)
        {
            emit errorOccurred(currentFiPair_, e.what());
            continue;
        }

        stats_.totalEntries++;
        tryUpdateProgress();

        auto it = entryIdToGroupIdxMap_.find(id);
        if (it == entryIdToGroupIdxMap_.end())
        {
            QFileInfoList list(1, entry);
            entryGroups_.append(std::move(list));
            entryIdToGroupIdxMap_.insert(id, entryGroups_.size() - 1);
        }
        else
        {
            size_t idx = it.value();
            entryGroups_[idx].append(entry);
        }
    }
}

void PatternLinkWorker::collectEntryGroups()
{
    for (const auto& dir : dirs_)
        collectEntryGroups(dir);
    entryIdToGroupIdxMap_.clear();
    scannedEntryKeys_.clear();
}
