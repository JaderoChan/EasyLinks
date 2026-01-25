#pragma once

#include <qstringlist.h>
#include <qthread.h>
#include <qobject.h>

#include "worker.h"
#include "progress_widget.h"
#include "link_config.h"
#include "types.h"

class FileLinkManager : public QObject
{
    Q_OBJECT

public:
    explicit FileLinkManager(
        LinkType linkType,
        const QStringList& sourcePaths,
        const QString& targetDir,
        const LinkConfig& config = LinkConfig(),
        QObject* parent = nullptr);
    ~FileLinkManager();

    void start();

signals:
    void cancel();
    void operate();

private:
    QThread workerThread_;
    LinkConfig config_;
    FileLinkWorker* worker_ = nullptr;
    ProgressWidget* progress_ = nullptr;
};
