#include "progress_dialog.h"

#include <qdir.h>

#include <easy_translate.hpp>

#include "conflict_decision_dialog.h"

#define CLSNAME "ProgressDialog"

ProgressDialog::ProgressDialog(
    LinkType linkType,
    const QString& sourceDir,
    const QString& targetDir,
    QWidget* parent)
    : QDialog(parent),
    errorLogDlg_(new ErrorLogDialog(this)),
    linkType_(linkType)
{
    ui.setupUi(this);
    setFixedSize(width(), height());

    ui.sourcePathText->setText(
        QString(
            "<html><head/><body><p><a href='%1'>"
            "<span style='color:rgba(0, 100, 180, 216); text-decoration:none;'>"
            "%2</span></a></p></body></html>")
        .arg(sourceDir, QDir(sourceDir).isRoot() ? sourceDir : QDir(sourceDir).dirName())
    );
    ui.targetPathText->setText(
        QString(
            "<html><head/><body><p><a href='%1'>"
            "<span style='color:rgba(0, 100, 180, 216); text-decoration:none;'>"
            "%2</span></a></p></body></html>")
        .arg(targetDir, QDir(targetDir).isRoot() ? targetDir : QDir(targetDir).dirName())
    );

    ui.errorWgt->setEnabled(false);
    errorLogDlg_->hide();
    pageToMainWidget();

    speedRemainingTimeUpdateTimer_.setInterval(1000);
    connect(&speedRemainingTimeUpdateTimer_, &QTimer::timeout, this, &ProgressDialog::updateSpeedRemainingTimeDisplay);
    speedRemainingTimeUpdateTimer_.start();

    connect(ui.pauseResumeBtn, &QPushButton::clicked, this, &ProgressDialog::onPauseResumeBtnPressed);
    connect(ui.cancelBtn, &QPushButton::clicked, this, &ProgressDialog::onCancelBtnPressed);
    connect(ui.detailsBtn, &QPushButton::clicked, this, &ProgressDialog::onDetailsBtnPressed);
    connect(ui.skipAllBtn, &QPushButton::clicked, this, &ProgressDialog::onSkipAllBtnPressed);
    connect(ui.replaceAllBtn, &QPushButton::clicked, this, &ProgressDialog::onReplaceAllBtnPressed);
    connect(ui.keepAllBtn, &QPushButton::clicked, this, &ProgressDialog::onKeepAllBtnPressed);
    connect(ui.decideAllBtn, &QPushButton::clicked, this, &ProgressDialog::onDecideAllBtnPressed);

    updatePauseResumeBtnIcon();
    updateSpeedRemainingTimeDisplay();
    updateCurrentEntryDisplay(EntryPair());
    updateText();
}

ProgressDialog::~ProgressDialog()
{
    cancel();
}

void ProgressDialog::pause()
{
    paused_ = true;
    updatePauseResumeBtnIcon();
    emit pauseTriggered();
}

void ProgressDialog::resume()
{
    paused_ = false;
    updatePauseResumeBtnIcon();
    emit resumeTriggered();
}

void ProgressDialog::cancel()
{
    emit cancelTriggered();
}

void ProgressDialog::updateProgress(const EntryPair& currentEntryPair, const LinkStats& stats)
{
    stats_ = stats;
    updateCurrentEntryDisplay(currentEntryPair);
    updateStatsDisplay();
}

void ProgressDialog::appendErrorLog(LinkType linkType, const EntryPair& entryPair, const QString& errorMsg)
{
    errorLogDlg_->appendLog(linkType, entryPair, errorMsg);
}

void ProgressDialog::decideConflicts(const LinkTasks& conflicts)
{
    conflicts_ = conflicts;
    pageToECSWidget();
}

void ProgressDialog::onWorkFinished()
{
    if (stats_.failedEntries == 0)
    {
        close();
        return;
    }

    speedRemainingTimeUpdateTimer_.stop();
    ui.pauseResumeBtn->setDisabled(true);
    ui.cancelBtn->setDisabled(true);
    updateCurrentEntryDisplay(EntryPair());
    lastProcessedEntries_ = stats_.processedEntries;
    updateSpeedRemainingTimeDisplay();
}

void ProgressDialog::laterShow(int ms)
{
    auto timer = new QTimer(this);
    timer->setInterval(ms);
    connect(timer, &QTimer::timeout, this, &QDialog::show);
    connect(timer, &QTimer::timeout, timer, &QObject::deleteLater);
}

void ProgressDialog::updateText()
{
    setWindowTitle(EASYTR(CLSNAME ".WindowTitle"));
    updateHeaderText1();
    ui.headerText2->setText(EASYTR(CLSNAME ".Label.HeaderText2"));
    ui.speedText->setText(EASYTR(CLSNAME ".Label.Speed"));
    ui.speedUnitText->setText(EASYTR(CLSNAME ".Label.SpeedUnit"));
    ui.currentEntryText->setText(EASYTR(CLSNAME ".Label.CurrentEntry"));
    updateCurrentEntryTypeText();
    ui.remainingTimeText->setText(EASYTR(CLSNAME ".Label.RemainingTime"));
    ui.approximatelyText->setText(EASYTR(CLSNAME ".Label.Approximately"));
    ui.hourUnitText->setText(EASYTR(CLSNAME ".Label.Hour"));
    ui.minUnitText->setText(EASYTR(CLSNAME ".Label.Min"));
    ui.secUnitText->setText(EASYTR(CLSNAME ".Label.Sec"));
    ui.remainingEntriesText->setText(EASYTR(CLSNAME ".Label.RemainingEntries"));
    ui.failedEntriesText->setText(EASYTR(CLSNAME ".Label.FailedEntries"));
    ui.detailsBtn->setText(EASYTR(CLSNAME ".Button.Details"));
    updateECSWidgetTipText();
    ui.skipAllBtn->setText(EASYTR(CLSNAME ".Button.SkipAll"));
    ui.replaceAllBtn->setText(EASYTR(CLSNAME ".Button.ReplaceAll"));
    ui.keepAllBtn->setText(EASYTR(CLSNAME ".Button.KeepAll"));
    ui.keepAllBtn->setToolTip(EASYTR(CLSNAME ".Button.KeepAll.ToolTip"));
    ui.decideAllBtn->setText(EASYTR(CLSNAME ".Button.DecideAll"));
}

void ProgressDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        updateText();
    QDialog::changeEvent(event);
}

void ProgressDialog::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        // 禁用键入ESC关闭对话框功能。
        case Qt::Key_Escape:
            break;
        default:
            QDialog::keyPressEvent(event);
            break;
    }
}

void ProgressDialog::onPauseResumeBtnPressed()
{
    paused_ ? resume() : pause();
}

void ProgressDialog::onCancelBtnPressed()
{
    cancel();
}

void ProgressDialog::onDetailsBtnPressed()
{
    if (!errorLogDlg_->isVisible())
        errorLogDlg_->show();
    errorLogDlg_->activateWindow();
    errorLogDlg_->raise();
}

void ProgressDialog::onReplaceAllBtnPressed()
{
    pageToMainWidget();
    emit allConflictsDecided(ECS_REPLACE);
}

void ProgressDialog::onSkipAllBtnPressed()
{
    pageToMainWidget();
    emit allConflictsDecided(ECS_SKIP);
}

void ProgressDialog::onKeepAllBtnPressed()
{
    pageToMainWidget();
    emit allConflictsDecided(ECS_KEEP);
}

void ProgressDialog::onDecideAllBtnPressed()
{
    pageToMainWidget();
    ConflictDecisionDialog dlg(conflicts_, this);
    int ret = dlg.exec();
    if (ret == QDialog::Accepted)
    {
        // 如果所有冲突项都使用None/Skip策略则直接发送“对所有冲突项采用Skip策略”信号，以进行优化。
        if (normalizeECS(conflicts_))
            emit allConflictsDecided(ECS_SKIP);
        else
            emit conflictsDecided(conflicts_);
    }
    else
    {
        // 默认对所有冲突项采用Skip策略。
        emit allConflictsDecided(ECS_SKIP);
    }
}

bool ProgressDialog::normalizeECS(LinkTasks& tasks)
{
    int counter = 0;
    for (auto& task : tasks)
    {
        if (task.ecs == ECS_NONE || task.ecs == ECS_SKIP)
        {
            task.ecs = ECS_SKIP;
            counter++;
        }
    }
    return counter == stats_.conflicts;
}

void ProgressDialog::updateStatsDisplay()
{
    updateProgressDisplay();
    updateRemainingEntriesDisplay();
    updateFailedCountDisplay();
    if (lastTotalEntries_ != stats_.totalEntries)
    {
        lastTotalEntries_ = stats_.totalEntries;
        updateHeaderText1();
    }
}

void ProgressDialog::updateProgressDisplay()
{
    ui.progressBar->setValue(stats_.progress());
}

void ProgressDialog::updateSpeedRemainingTimeDisplay()
{
    speed_ = (stats_.processedEntries - lastProcessedEntries_);
    lastProcessedEntries_ = stats_.processedEntries;
    ui.speedValue->setText(QString::number(speed_));

    // 保证数字具有两位，使用0进行左补位。
    static auto formatNumberString = [](int num) -> QString
    { return (num < 10 ? "0" : "") + QString::number(num); };

    int remainingTime = (speed_ == 0 ? 0 : (stats_.totalEntries - stats_.processedEntries) / speed_);
    if (speed_ == 0)
    {
        ui.remainingTimeHourValue->setText("-");
        ui.remainingTimeMinValue->setText("-");
        ui.remainingTimeSecValue->setText("-");
    }
    else
    {
        int sec = remainingTime % 60;
        int min = ((remainingTime - sec) / 60) % 60;
        int hour = remainingTime / 3600;
        ui.remainingTimeHourValue->setText(formatNumberString(hour));
        ui.remainingTimeMinValue->setText(formatNumberString(min));
        ui.remainingTimeSecValue->setText(formatNumberString(sec));
    }
}

void ProgressDialog::updateCurrentEntryDisplay(const EntryPair& currentEntryPair)
{
    const auto& source = currentEntryPair.source;
    ui.currentEntryValue->setText(source.absoluteFilePath());
    ui.currentEntryValue->setToolTip(source.absoluteFilePath());

    ui.fileText->hide();
    ui.directoryText->hide();
    ui.symbolText->hide();

    if (source.isSymbolicLink() || source.isShortcut() || source.isBundle() || source.isJunction())
        ui.symbolText->show();
    else if (source.isFile())
        ui.fileText->show();
    else if (source.isDir())
        ui.directoryText->show();
    else ; // pass
}

void ProgressDialog::updateRemainingEntriesDisplay()
{
    ui.remainingEntriesValue->setText(QString::number(stats_.totalEntries - stats_.processedEntries));
}

void ProgressDialog::updateFailedCountDisplay()
{
    ui.failedEntriesValue->setText(QString::number(stats_.failedEntries));
    // 如果失败项数量不为0则启用errorWgt。
    if (stats_.failedEntries > 0 && !ui.errorWgt->isEnabled())
        ui.errorWgt->setEnabled(true);
}

void ProgressDialog::pageToMainWidget()
{
    ui.stackedWidget->setCurrentIndex(0);
}

void ProgressDialog::pageToECSWidget()
{
    ui.stackedWidget->setCurrentIndex(1);
    updateECSWidgetTipText();   // 显式更新Tip文本以同步当前冲突项数量。
    ui.replaceAllBtn->setFocus();
    ui.replaceAllBtn->setShortcut(QKeySequence::fromString("R"));
    ui.skipAllBtn->setShortcut(QKeySequence::fromString("S"));
    ui.keepAllBtn->setShortcut(QKeySequence::fromString("K"));
    ui.decideAllBtn->setShortcut(QKeySequence::fromString("D"));
    qApp->alert(this);          // 通过任务栏提醒用户需要决定冲突处理策略。
}

QString ProgressDialog::linkTypeString() const
{
    switch (linkType_)
    {
        case LT_HARDLINK:    return EASYTR("LinkType.Hardlink");
        case LT_SYMLINK:     return EASYTR("LinkType.Symlink");
        default: return "";
    }
}

void ProgressDialog::updateHeaderText1()
{
    ui.headerText1->setText(
        QString("%1 %2 %3").arg(
            linkTypeString(),
            QString::number(stats_.totalEntries),
            EASYTR(CLSNAME ".Label.HeaderText1")
        )
    );
}

void ProgressDialog::updatePauseResumeBtnIcon()
{
    if (paused_)
        ui.pauseResumeBtn->setIcon(QIcon(":/icons/play.ico"));
    else
        ui.pauseResumeBtn->setIcon(QIcon(":/icons/pause.ico"));
}

void ProgressDialog::updateCurrentEntryTypeText()
{
    ui.fileText->setText(EASYTR(CLSNAME ".EntryType.File"));
    ui.directoryText->setText(EASYTR(CLSNAME ".EntryType.Dir"));
    ui.symbolText->setText(EASYTR(CLSNAME ".EntryType.Symbol"));
}

void ProgressDialog::updateECSWidgetTipText()
{
    ui.ecsTipText->setText(QString(EASYTR(CLSNAME ".Label.ECSTipText")).arg(stats_.conflicts));
}
