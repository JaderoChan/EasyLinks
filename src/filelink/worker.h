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

    // 比较两个路径是否位于同一驱动器下。
    static bool isOnSameDriver(const QString& a, const QString& b);
    // 如果目标路径不存在，会尝试创建其所有父路径。
    // 会判断source与target是否是同一个文件实体。
    // 对于符号链接，会根据source指向的条目是file还是directory创建对应的符号链接。
    // 对于硬链接，断言source指向的条目一定是文件。
    static void createLink(LinkType linkType, QFileInfo source, QFileInfo target);

    void run();
    void setParameters(LinkType linkType, const QStringList& sourcePaths, const QString& targetDir);
    // 下面两个函数都应该在工作线程处于暂停状态时被调用
    // 应用至全部。
    // 线程安全
    void setConflictsDecisionForAll(EntryConflictStrategy ecs);
    // 单独决定每个。
    // 线程安全
    void setConflictsDecision(const LinkTasks& tasks);

    // 线程安全
    void pause();
    void resume();
    void cancel();

signals:
    void progressUpdated(EntryPair currentEntryPair, LinkStats stats);
    void errorOccurred(LinkType linkType, EntryPair entryPair, QString errorMsg);
    void conflictsDecisionWaited(LinkTasks tasks);
    void finished();

protected:
    // 若当前操作已取消返回true；若当前操作处于暂停，函数将阻塞，直到暂停解除。
    // 对于后者而言，其返回值取决于暂停解除后操作是否已取消，若是，返回true，否则返回false。
    bool processPauseAndCancel();
    // 根据参数构建一个任务并将其添加至任务队列。(冲突处理策略使用None，意味着由用户决定)
    void addTask(LinkType linkType, QFileInfo sourceEntry, QFileInfo targetEntry);
    // 仅当Conflict Policy是None且发生冲突时返回true（意味着等待用户决定），否则返回false。
    bool createLink(LinkType linkType, QFileInfo source, QFileInfo target, EntryConflictStrategy ecs);
    // 处理任务队列，返回待用户确认的任务列表。
    LinkTasks processTasks();
    // 尝试发射进度更新信号。
    // 具体而言，如果距离上次发射信号达到既定时长则会发射信号，否则什么都不做。
    // 如果指定了参数forced为true则会忽略间隔时长强制发射信号。
    void tryUpdateProgress(bool forced = false);

private:
    void waitConflictsDecision();

    // 收集条目并创建任务。
    // 对于Symlink分支，非递归收集原路径列表中的子目录与文件。
    // 对于Hardlink分支，递归收集原路径列表中的所有文件及其子目录的文件。
    void collectEntriesForSymlink();
    void collectEntriesForHardlinkHelper(const QString& sourcePath, const QString& targetDir);
    void collectEntriesForHardlink();
    void collectEntries();

    QFileInfo generateNewPath(QFileInfo file);

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
    std::atomic<bool> ECSApplyToAll_{false};
    std::atomic<EntryConflictStrategy> ECSOfAll{ECS_NONE};

    std::atomic<bool> removeToTrash_{false};

    EntryPair currentEntryPair_;
    QElapsedTimer progressUpdateTimer_;
};
