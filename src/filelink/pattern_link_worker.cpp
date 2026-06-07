#include "pattern_link_worker.h"

#include <mutex>
#include <stdexcept>

#include <qdir.h>
#include <qhash.h>

#include <utils/file_io.h>
#include "utils.h"

#define THROW_RTERR(errorMsg) throw std::runtime_error(errorMsg)

constexpr unsigned int PROGRESS_UPDATE_INTERVAL_MS = 20;

PatternLinkWorker::PatternLinkWorker(QObject* parent)
    : QObject(parent)
{}

PatternLinkWorker::~PatternLinkWorker()
{
    cancel();
}

void PatternLinkWorker::setParameters(const QStringList& dirList, Patterns patterns, bool needReview)
{
    dirs_       = dirList;
    patterns_   = patterns;
    needReview_ = needReview;
}

void PatternLinkWorker::run()
{
    progressUpdateTimer_.restart();

    collectEntryGroups();
    tryUpdateProgress(true);

    if (needReview_)
    {
        emit reviewRequested(entryGroups_);
        pause();
    }

    if (processPauseAndCancel())
    {
        emit finished();
        return;
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
        emit progressUpdated(currentEntry_, stats_);
    }
    else if (progressUpdateTimer_.elapsed() > PROGRESS_UPDATE_INTERVAL_MS)
    {
        progressUpdateTimer_.restart();
        emit progressUpdated(currentEntry_, stats_);
    }
}

void PatternLinkWorker::work()
{
    for (auto& entrys : entryGroups_)
    {
        if (entrys.empty())
            continue;

        auto master = entrys.front();
        entrys.pop_front();
        while (!entrys.empty())
        {
            if (processPauseAndCancel())
                return;

            currentEntry_ = entrys.front();
            entrys.pop_front();
            stats_.processedEntries++;

            try
            {
                currentEntry_.refresh();
                if (currentEntry_.isFile() || isWindowsSymlink(currentEntry_))
                {
                    bool ok = (removeToTrash_ ?
                        QFile::moveToTrash(currentEntry_.filePath()) :
                        QFile::remove(currentEntry_.filePath()));
                    if (!ok)
                        THROW_RTERR("Failed to remove the file.");
                    createLink(LT_HARDLINK, currentEntry_, master);
                }
                else
                {
                    THROW_RTERR("The target file does nt exist or it's a directory.");
                }
            }
            catch (std::exception& e)
            {
                stats_.failedEntries++;
                emit errorOccurred(currentEntry_, e.what());
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
            collectEntryGroups(entry.path());

        currentEntry_ = entry;
        stats_.totalEntries++;
        tryUpdateProgress();

        const uint64_t id = makeEntryId(entry, patterns_);
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
}
