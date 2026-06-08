#include "controller.h"

#include <qdir.h>
#include <qfileinfo.h>

#include "utils/logging.h"
#include <app/file_review_dialog.h>

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

    linkType_ = linkType;
    targetDir_ = targetDir;

    worker_ = new FileLinkWorker();
    worker_->setParameters(linkType, sourcePaths, targetDir, config_.renamePattern, config_.removeToTrash);
    worker_->moveToThread(&workerThread_);

    auto sourceDir = QFileInfo(sourcePaths.front()).absolutePath();
    progress_ = new ProgressWidget(linkType, sourceDir, targetDir, config_.keepDialogOnErrorOccurred);
    progress_->setAttribute(Qt::WA_DeleteOnClose);

    debugOut(qInfo(), "[FileLink] %1 %2 entries in %3 to %4 directory.",
        linkType == LinkType::LT_SYMLINK ? "Creating symbolic links for" : "Creating hard links for",
        QString::number(sourcePaths.size()),
        sourceDir,
        targetDir);

    connect(worker_, &FileLinkWorker::progressUpdated, progress_, &ProgressWidget::updateProgress);
    connect(worker_, &FileLinkWorker::progressUpdated, this, [this](const EntryPair&, const LinkStats& stats)
    { lastStats_ = stats; });
    connect(worker_, &FileLinkWorker::errorOccurred, progress_, [this](LinkType lt, const EntryPair& ep, const QString& em)
    {
        progress_->appendErrorLog(lt, ep, em);
    });
    connect(worker_, &FileLinkWorker::conflictsDecisionWaited, progress_, &ProgressWidget::decideConflicts);
    connect(worker_, &FileLinkWorker::finished, progress_, &ProgressWidget::onWorkFinished);
    connect(worker_, &FileLinkWorker::finished, this, [this]()
    { emit linkFinished(linkType_, targetDir_, lastStats_); });
    connect(worker_, &FileLinkWorker::finished, &workerThread_, &QThread::quit);

    connect(progress_, &ProgressWidget::pauseTriggered, worker_, &FileLinkWorker::pause, Qt::DirectConnection);
    connect(progress_, &ProgressWidget::resumeTriggered, worker_, &FileLinkWorker::resume, Qt::DirectConnection);
    connect(progress_, &ProgressWidget::cancelTriggered, worker_, &FileLinkWorker::cancel, Qt::DirectConnection);

    connect(progress_, &ProgressWidget::conflictsDecided, worker_, &FileLinkWorker::setConflictsDecision, Qt::DirectConnection);
    connect(progress_, &ProgressWidget::allConflictsDecided, worker_, &FileLinkWorker::setConflictsDecisionForAll, Qt::DirectConnection);

    connect(this, &FileLinkController::operate, worker_, &FileLinkWorker::run);
    connect(&workerThread_, &QThread::finished, this, &QObject::deleteLater);
}

FileLinkController::FileLinkController(
    const QStringList& dirs, Patterns patterns, bool needReview, const LinkConfig& config, QObject* parent)
    : QObject(parent), config_(config)
{
    if (dirs.empty())
        return;

    patWorker_ = new PatternLinkWorker();
    patWorker_->setParameters(dirs, patterns, needReview, config.removeToTrash);
    patWorker_->moveToThread(&workerThread_);

    progress_ = new ProgressWidget(dirs, config.keepDialogOnErrorOccurred);
    progress_->setAttribute(Qt::WA_DeleteOnClose);

    debugOut(qInfo(), "[Pattern Link] Pattern link %1 directories%3 use pattern %2.",
        dirs.size(), patterns, dirs.size() == 1 ? (" (" + dirs.constFirst() + ")") : "");

    connect(patWorker_, &PatternLinkWorker::progressUpdated, progress_, &ProgressWidget::updatePatternLinkProgress);
    connect(patWorker_, &PatternLinkWorker::progressUpdated, this, [this](const FileInfoPair&, const LinkStats& stats)
    { lastStats_ = stats; });
    connect(patWorker_, &PatternLinkWorker::errorOccurred, progress_, [this](FileInfoPair fiPair, const QString& errorMsg)
    {
        progress_->appendErrorLog(fiPair, errorMsg);
    });
    connect(patWorker_, &PatternLinkWorker::reviewRequested, this, [this](QList<QFileInfoList>* entryGroups)
    {
        if (!entryGroups)
            return;

        auto reviewDlg = new FileReviewDialog(*entryGroups);
        reviewDlg->setAttribute(Qt::WA_DeleteOnClose);
        connect(reviewDlg, &FileReviewDialog::reviewFinished, this, [this]() { patWorker_->finishReview(); });
        reviewDlg->exec();
    });
    connect(patWorker_, &PatternLinkWorker::finished, progress_, &ProgressWidget::onWorkFinished);
    connect(patWorker_, &PatternLinkWorker::finished, this, [this]()
    { emit patternLinkFinished(lastStats_); });
    connect(patWorker_, &PatternLinkWorker::finished, &workerThread_, &QThread::quit);

    connect(progress_, &ProgressWidget::pauseTriggered, patWorker_, &PatternLinkWorker::pause, Qt::DirectConnection);
    connect(progress_, &ProgressWidget::resumeTriggered, patWorker_, &PatternLinkWorker::resume, Qt::DirectConnection);
    connect(progress_, &ProgressWidget::cancelTriggered, patWorker_, &PatternLinkWorker::cancel, Qt::DirectConnection);

    connect(this, &FileLinkController::operate, patWorker_, &PatternLinkWorker::run);
    connect(&workerThread_, &QThread::finished, this, &QObject::deleteLater);
}

FileLinkController::~FileLinkController()
{
    stop();
    if (workerThread_.isRunning())
        workerThread_.wait();
    if (worker_)
        delete worker_;
    if (patWorker_)
        delete patWorker_;
}

void FileLinkController::start()
{
    if ((worker_ || patWorker_) && progress_)
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
    if (patWorker_)
        patWorker_->cancel();
}
