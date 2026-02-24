#include "controller.h"

#include <qdir.h>
#include <qfileinfo.h>

#include "utils/logging.h"

FileLinkController::FileLinkController(
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
    worker_->setParameters(linkType, sourcePaths, targetDir, config_.renamePattern, config_.removeToTrash);
    worker_->moveToThread(&workerThread_);

    auto sourceDir = QFileInfo(sourcePaths.front()).absolutePath();
    progress_ = new ProgressWidget(linkType, sourceDir, targetDir, config_.keepDialogOnErrorOccurred);
    progress_->setAttribute(Qt::WA_DeleteOnClose);

    qout(qInfo(), "[FileLink] %1 %2 entries in %3 to %4 directory.",
        linkType == LinkType::LT_SYMLINK ? "Creating symbolic links for" : "Creating hard links for",
        QString::number(sourcePaths.size()),
        sourceDir,
        targetDir);

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

    connect(this, &FileLinkController::operate, worker_, &FileLinkWorker::run);
    connect(&workerThread_, &QThread::finished, this, &QObject::deleteLater);
}

FileLinkController::~FileLinkController()
{
    stop();
    if (workerThread_.isRunning())
        workerThread_.wait();
    if (worker_)
        delete worker_;
}

void FileLinkController::start()
{
    if (worker_ && progress_)
    {
        progress_->laterShowAndActivate(200);
        workerThread_.start();
        emit operate();
    }
}

void FileLinkController::stop()
{
    if (worker_)
        worker_->cancel();
}
