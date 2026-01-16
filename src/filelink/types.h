#pragma once

#include <qfileinfo.h>
#include <qdatetime.h>
#include <qqueue.h>

enum LinkType : char
{
    LT_SYMLINK,
    LT_HARDLINK
};

// 冲突（目标已存在时）处理策略
enum ConflictingEntryStrategy : char
{
    CES_NONE,       // 无策略
    CES_REPLACE,    // 覆盖同名条目
    CES_SKIP,       // 跳过同名条目
    CES_KEEP        // 均保留（通过重命名）
};

struct Entry
{
    QFileInfo fileinfo;
    struct {
        QDateTime lastModified;
        qint64 size;
    } preInfo; // 用于ConflictDecision相关界面显示的预获取信息，用于提升ConflictDecisionDialog界面的表格显示性能。

    void updatePreInfo()
    {
        preInfo = {
            fileinfo.lastModified(),
            fileinfo.size()
        };
    }
};

struct EntryPair
{
    Entry source;
    Entry target;
};

struct LinkTask
{
    LinkType linkType;
    EntryPair entryPair;
    ConflictingEntryStrategy ces;
};

using LinkTasks = QQueue<LinkTask>;

struct LinkStats
{
    int totalEntries = 0;
    int processedEntries = 0;
    int successfulEntries = 0;
    int failedEntries = 0;
    int conflicts = 0;

    double progress() const
    { return totalEntries > 0 ? processedEntries * 100.0 / totalEntries : 0.0; }
};
