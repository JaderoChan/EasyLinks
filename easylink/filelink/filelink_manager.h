#pragma once

#include <qstringlist.h>
#include <qthread.h>
#include <qobject.h>

#include "worker.h"
#include "progress_dialog.h"
#include "types.h"

class FileLinkManager : public QObject
{
    Q_OBJECT

public:
    explicit FileLinkManager(QObject* parent = nullptr);
    ~FileLinkManager();

    void createLinks(LinkType linkType, const QStringList& sourcePaths, const QString& targetDir);

signals:
    void cancel();
    void operate();

private:
    QThread workerThread_;
    FileLinkWorker* worker_ = nullptr;
    ProgressDialog* dlg_ = nullptr;
};
