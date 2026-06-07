#pragma once

#include <atomic>

#include <qfileinfo.h>
#include <qstringlist.h>
#include <qobject.h>
#include <qelapsedtimer.h>

#include "types.h"
#include "pattern.h"

/**
 * @brief 模式匹配文件硬链接工作。
 * @details
 * 对于给定的目录列表与模式规则，首先递归遍历所有给定目录及其子目录，并根据规则筛选得到符合模式的文件组的列表。
 * 列表中的每个文件组也是一个列表，每个文件组中的文件都是符合模式且待链接的。
 */
class PatternLinkWorker : public QObject
{
    Q_OBJECT

public:
    explicit PatternLinkWorker(QObject* parent = nullptr);
    ~PatternLinkWorker();

    void setParameters(const QStringList& dirList, Patterns patterns, bool needReview);
    void run();

    void pause();
    void resume();
    void cancel();
    void finishReview();

signals:
    void progressUpdated(QFileInfo currentEntry, LinkStats stats);
    void reviewRequested(QList<QFileInfoList>& entryGroups);
    void errorOccurred(QFileInfo entry, QString errorMsg);
    void finished();

protected:
    bool processPauseAndCancel();
    void tryUpdateProgress(bool forced = false);
    void work();

private:
    void collectEntryGroups(const QString& dir);
    void collectEntryGroups();

    QStringList             dirs_;
    Patterns                patterns_;
    bool                    needReview_;

    QList<QFileInfoList>    entryGroups_;
    QMap<uint64_t, size_t>  entryIdToGroupIdxMap_;
    LinkStats               stats_;

    bool                    paused_ = false;
    mutable std::mutex      pausedMtx_;
    std::condition_variable pausedCond_;
    std::atomic<bool>       cancelled_{false};

    bool                    removeToTrash_ = false;

    QFileInfo               currentEntry_;
    QElapsedTimer           progressUpdateTimer_;
};
