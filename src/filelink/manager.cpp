#include "manager.h"

#include <qdir.h>
#include <qfileinfo.h>

FileLinkManager::FileLinkManager(
    LinkType linkType,
    const QStringList& sourcePaths,
    const QString& targetDir,
    const LinkConfig& config,
    QObject* parent)
    : QObject(parent), config_(config)
{
    if (sourcePaths.isEmpty())
        return;

    worker_ = new FileLinkWorker();
    worker_->setParameters(linkType, sourcePaths, targetDir, config_.removeToTrash);
    worker_->moveToThread(&workerThread_);

    auto sourceDir = QFileInfo(sourcePaths.front()).absolutePath();
    progress_ = new ProgressWidget(linkType, sourceDir, targetDir, config_.keepDialogWhenErrorOccurred);
    progress_->setAttribute(Qt::WA_DeleteOnClose);

    connect(worker_, &FileLinkWorker::progressUpdated, progress_, &ProgressWidget::updateProgress);
    connect(worker_, &FileLinkWorker::errorOccurred, progress_, &ProgressWidget::appendErrorLog);
    connect(worker_, &FileLinkWorker::conflictsDecisionWaited, progress_, &ProgressWidget::decideConflicts);
    connect(worker_, &FileLinkWorker::finished, progress_, &ProgressWidget::onWorkFinished);
    connect(worker_, &FileLinkWorker::finished, &workerThread_, &QThread::quit);

    connect(progress_, &ProgressWidget::pauseTriggered, worker_, &FileLinkWorker::pause, Qt::DirectConnection);
    connect(progress_, &ProgressWidget::resumeTriggered, worker_, &FileLinkWorker::resume, Qt::DirectConnection);
    connect(progress_, &ProgressWidget::cancelTriggered, worker_, &FileLinkWorker::cancel, Qt::DirectConnection);

    connect(progress_, &ProgressWidget::conflictsDecided, worker_, &FileLinkWorker::setConflictsDecision, Qt::DirectConnection);
    connect(progress_, &ProgressWidget::allConflictsDecided, worker_, &FileLinkWorker::setConflictsDecisionForAll, Qt::DirectConnection);

    connect(this, &FileLinkManager::operate, worker_, &FileLinkWorker::run);
    connect(this, &FileLinkManager::cancel, worker_, &FileLinkWorker::cancel, Qt::DirectConnection);
    connect(&workerThread_, &QThread::finished, worker_, &QObject::deleteLater);
}

FileLinkManager::~FileLinkManager()
{
    emit cancel();
    workerThread_.quit();
    workerThread_.wait();
}

void FileLinkManager::start()
{
    if (worker_ && progress_)
    {
        progress_->show();    // for dev
        // progress_->laterShow(200);
        workerThread_.start();

        emit operate();
    }
}
