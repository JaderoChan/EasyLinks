#pragma once

#include <qfileinfo.h>
#include <qqueue.h>

#define URL_ROLE (Qt::UserRole + 1)

enum LinkType : char
{
    LT_SYMLINK,
    LT_HARDLINK
};

// 冲突（目标已存在时）处理策略
enum EntryConflictStrategy : char
{
    ECS_NONE,       // 无策略
    ECS_REPLACE,    // 覆盖同名条目
    ECS_SKIP,       // 跳过同名条目
    ECS_KEEP        // 均保留（通过重命名）
};

struct EntryPair
{
    QFileInfo source;
    QFileInfo target;
};

struct LinkTask
{
    LinkType linkType;
    EntryPair entryPair;
    EntryConflictStrategy ecs;
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
