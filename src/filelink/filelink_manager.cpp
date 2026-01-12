#include "filelink_manager.h"

#include <qdir.h>
#include <qfileinfo.h>

FileLinkManager::FileLinkManager(QObject* parent)
    : QObject(parent), worker_(new FileLinkWorker())
{}

FileLinkManager::~FileLinkManager()
{
    qDebug() << "~FileLinkManager";
    emit cancel();
    workerThread_.quit();
    workerThread_.wait();
}

void FileLinkManager::createLinks(LinkType linkType, const QStringList& sourcePaths, const QString& targetDir)
{
    if (sourcePaths.isEmpty())
        return;

    worker_ = new FileLinkWorker();
    worker_->setParameters(linkType, sourcePaths, targetDir);
    worker_->moveToThread(&workerThread_);

    {
        auto sourceDir = QFileInfo(sourcePaths.front()).absolutePath();
        dlg_ = new ProgressDialog(linkType, sourceDir, targetDir);
        dlg_->setAttribute(Qt::WA_DeleteOnClose);
    }

    connect(worker_, &FileLinkWorker::progressUpdated, dlg_, &ProgressDialog::updateProgress);
    connect(worker_, &FileLinkWorker::errorOccurred, dlg_, &ProgressDialog::appendErrorLog);
    connect(worker_, &FileLinkWorker::conflictsDecisionWaited, dlg_, &ProgressDialog::decideConflicts);
    connect(worker_, &FileLinkWorker::finished, dlg_, &ProgressDialog::onWorkFinished);
    connect(worker_, &FileLinkWorker::finished, &workerThread_, &QThread::quit);

    connect(dlg_, &ProgressDialog::pauseTriggered, worker_, &FileLinkWorker::pause, Qt::DirectConnection);
    connect(dlg_, &ProgressDialog::resumeTriggered, worker_, &FileLinkWorker::resume, Qt::DirectConnection);
    connect(dlg_, &ProgressDialog::cancelTriggered, worker_, &FileLinkWorker::cancel, Qt::DirectConnection);

    connect(dlg_, &ProgressDialog::conflictsDecided, worker_, &FileLinkWorker::setConflictsDecision, Qt::DirectConnection);
    connect(dlg_, &ProgressDialog::allConflictsDecided, worker_, &FileLinkWorker::setConflictsDecisionForAll, Qt::DirectConnection);

    connect(this, &FileLinkManager::operate, worker_, &FileLinkWorker::run);
    connect(this, &FileLinkManager::cancel, worker_, &FileLinkWorker::cancel, Qt::DirectConnection);
    connect(&workerThread_, &QThread::finished, worker_, &QObject::deleteLater);

    dlg_->show();    // for dev
    // dlg_->laterShow(200);
    workerThread_.start();

    emit operate();
}
