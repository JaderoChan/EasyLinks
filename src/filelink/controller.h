#pragma once

#include <qstringlist.h>
#include <qthread.h>
#include <qobject.h>

#include "types.h"
#include "link_config.h"
#include "worker.h"
#include "pattern_link_worker.h"
#include "progress_widget.h"

class FileLinkController : public QObject
{
    Q_OBJECT

public:
    FileLinkController(
        LinkType linkType,
        const QStringList& sourcePaths,
        const QString& targetDir,
        const LinkConfig& config = LinkConfig(),
        QObject* parent = nullptr);
    FileLinkController(
        const QStringList& dirs,
        Patterns patterns,
        bool needReview,
        const LinkConfig& config = LinkConfig(),
        QObject* parent = nullptr);
    ~FileLinkController();

    void start();
    void stop();

signals:
    void operate();
    void linkFinished(LinkType linkType, QString targetDir, LinkStats stats);
    void patternLinkFinished(LinkStats stats);

private:
    QThread workerThread_;
    LinkConfig config_;
    FileLinkWorker* worker_ = nullptr;
    PatternLinkWorker* patWorker_ = nullptr;
    ProgressWidget* progress_ = nullptr;

    LinkType linkType_ = LT_SYMLINK;
    QString targetDir_;
    LinkStats lastStats_;
};
