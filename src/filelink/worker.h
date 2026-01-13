#pragma once

#include <atomic>
#include <condition_variable>

#include <qstringlist.h>
#include <qobject.h>
#include <qelapsedtimer.h>

#include "types.h"

class FileLinkWorker : public QObject
{
    Q_OBJECT

public:
    explicit FileLinkWorker(QObject* parent = nullptr);
    ~FileLinkWorker();

    // 比较所给路径是否位于同一驱动器下。
    static bool isOnSameDriver(const QString& a, const QString& b);
    // 判断所给fileinfo引用的文件实体是否为Shortcut、Junction。
    static bool isWindowsSymlink(const QFileInfo& fileinfo);
    // - 如果目标路径不存在，会尝试创建其所有父路径。
    // - 会判断source与target是否是同一个文件实体。
    // - 对于符号链接，会根据source引用的文件实体是file还是directory创建相应的符号链接。
    // - 对于硬链接，断言source引用的文件实体一定是文件/符号链接。
    static void createLink(LinkType linkType, const QFileInfo& source, const QFileInfo& target);

    void run();
    void setParameters(
        LinkType linkType,
        const QStringList& sourcePaths,
        const QString& targetDir,
        bool removeToTrash = false);

    // - 必须在工作线程处于暂停状态时调用。
    // - 设置冲突策略并应用至全部项。
    // - 线程安全。
    void setConflictsDecisionForAll(EntryConflictStrategy ecs);
    // - 必须在工作线程处于暂停状态时调用。
    // - 线程安全。
    void setConflictsDecision(const LinkTasks& tasks);

    // 以下函数均线程安全。
    void pause();
    void resume();
    void cancel();

signals:
    void progressUpdated(EntryPair currentEntryPair, LinkStats stats);
    void errorOccurred(LinkType linkType, EntryPair entryPair, QString errorMsg);
    void conflictsDecisionWaited(LinkTasks tasks);
    void finished();

protected:
    // 1. 若用户已取消返回true。
    // 2. 若用户已暂停，函数将阻塞，直到暂停解除。
    // - 对于情况2而言，如果暂停的解除是由取消操作产生的则返回true，否则返回false（暂停的解除由用户恢复）。
    bool processPauseAndCancel();
    // 创建一个链接任务并将其添加至任务队列。(冲突处理策略使用None，即实际的冲突处理策略将由用户决定)
    void addTask(LinkType linkType, const QFileInfo& source, const QFileInfo& target);
    // - 仅当冲突处理策略是None且发生冲突时返回false，否则返回true。
    // - 传入的source/target可能发生改变。
    bool createLink(LinkType linkType, QFileInfo& source, QFileInfo& target, EntryConflictStrategy ecs);
    // 处理链接任务，返回冲突项。
    LinkTasks processTasks();
    // 尝试发射进度更新信号。
    // - 具体而言，如果距离上次发射信号达到指定时长则会发射信号，否则什么都不做。
    // - 如果指定了参数forced为true则会立即发射信号。
    void tryUpdateProgress(bool forced = false);

private:
    void waitConflictsDecision();

    // 收集条目并创建链接任务。
    // - 对于Symlink分支，其结果等同于原路径列表中的项。
    // - 对于Hardlink分支，收集原路径列表中的子文件，并递归收集原路径列表中目录的子文件。
    void collectEntriesForSymlink();
    void collectEntriesForHardlinkHelper(const QString& sourcePath, const QString& targetDir);
    void collectEntriesForHardlink();
    void collectEntries();

    void generateNewPath(QFileInfo& file);

private:
    // 断言路径均不使用'/'或'\'结尾。
    QStringList sourcePaths_;
    QString targetDir_;

    LinkType linkType_;
    LinkTasks tasks_;
    LinkStats stats_;

    std::condition_variable pausedCondition_;
    std::atomic<bool> paused_{false};
    std::atomic<bool> cancelled_{false};
    std::atomic<bool> ecsApplyToAll_{false};
    std::atomic<EntryConflictStrategy> ecsOfAll{ECS_NONE};

    bool removeToTrash_ = false;

    EntryPair currentEntryPair_;
    QElapsedTimer progressUpdateTimer_;
};
