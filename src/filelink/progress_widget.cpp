#include "progress_widget.h"

#include <qdir.h>

#include <easy_translate.hpp>

#include "conflict_decision_dialog.h"

#define CLSNAME "ProgressDialog"
#define PATH_TEXT_FORMAT_STRING \
"<html><head/><body><p><a href='file:///%1'><span style='color:rgba(0, 100, 180, 216); text-decoration:none;'>" \
"%2</span></a></p></body></html>"

ProgressWidget::ProgressWidget(
    LinkType linkType,
    const QString& sourceDir,
    const QString& targetDir,
    bool keepWhenErrorOccurred,
    QWidget* parent)
    : QWidget(parent),
    errorLogDlg_(this),
    linkType_(linkType),
    keepWhenErrorOccurred_(keepWhenErrorOccurred)
{
    ui.setupUi(this);
    setFixedSize(width(), height());

    ui.sourcePathText->setText(QString(PATH_TEXT_FORMAT_STRING).arg(
        sourceDir, QDir(sourceDir).isRoot() ? sourceDir : QDir(sourceDir).dirName()));
    ui.targetPathText->setText(QString(PATH_TEXT_FORMAT_STRING).arg(
        targetDir, QDir(targetDir).isRoot() ? targetDir : QDir(targetDir).dirName()));

    ui.errorWgt->setEnabled(false);
    errorLogDlg_.hide();
    pageToMainWidget();

    speedRemainingTimeUpdateTimer_.setInterval(1000);
    connect(&speedRemainingTimeUpdateTimer_, &QTimer::timeout, this, &ProgressWidget::updateSpeedRemainingTimeDisplay);
    speedRemainingTimeUpdateTimer_.start();

    connect(ui.pauseResumeBtn, &QPushButton::clicked, this, &ProgressWidget::onPauseResumeBtnPressed);
    connect(ui.cancelBtn, &QPushButton::clicked, this, &ProgressWidget::onCancelBtnPressed);
    connect(ui.detailsBtn, &QPushButton::clicked, this, &ProgressWidget::onDetailsBtnPressed);
    connect(ui.skipAllBtn, &QPushButton::clicked, this, &ProgressWidget::onSkipAllBtnPressed);
    connect(ui.replaceAllBtn, &QPushButton::clicked, this, &ProgressWidget::onReplaceAllBtnPressed);
    connect(ui.keepAllBtn, &QPushButton::clicked, this, &ProgressWidget::onKeepAllBtnPressed);
    connect(ui.decideAllBtn, &QPushButton::clicked, this, &ProgressWidget::onDecideAllBtnPressed);

    updatePauseResumeBtnIcon();
    updateSpeedRemainingTimeDisplay();
    updateCurrentEntryDisplay(EntryPair());
    updateText();
}

ProgressWidget::~ProgressWidget()
{
    cancel();
}

void ProgressWidget::pause()
{
    paused_ = true;
    updatePauseResumeBtnIcon();
    emit pauseTriggered();
}

void ProgressWidget::resume()
{
    paused_ = false;
    updatePauseResumeBtnIcon();
    emit resumeTriggered();
}

void ProgressWidget::cancel()
{
    emit cancelTriggered();
}

void ProgressWidget::updateProgress(const EntryPair& currentEntryPair, const LinkStats& stats)
{
    stats_ = stats;
    updateCurrentEntryDisplay(currentEntryPair);
    updateStatsDisplay();
}

void ProgressWidget::appendErrorLog(LinkType linkType, const EntryPair& entryPair, const QString& errorMsg)
{
    errorLogDlg_.appendLog(linkType, entryPair, errorMsg);
}

void ProgressWidget::decideConflicts(const LinkTasks& conflicts)
{
    conflicts_ = conflicts;
    showAndActivate();
    pageToEcsWidget();
}

void ProgressWidget::onWorkFinished()
{
    if (stats_.failedEntries == 0 || !keepWhenErrorOccurred_)
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

void ProgressWidget::showAndActivate()
{
    show();
    raise();
    activateWindow();
}

void ProgressWidget::laterShowAndActivate(int ms)
{
    auto timer = new QTimer(this);
    timer->start(ms);
    connect(timer, &QTimer::timeout, this, &ProgressWidget::showAndActivate);
    connect(timer, &QTimer::timeout, this, [=]() { delete timer; });
}

void ProgressWidget::updateText()
{
    setWindowTitle(linkTypeString());
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
    updateEcsWidgetTipText();
    ui.skipAllBtn->setText(EASYTR(CLSNAME ".Button.SkipAll"));
    ui.replaceAllBtn->setText(EASYTR(CLSNAME ".Button.ReplaceAll"));
    ui.keepAllBtn->setText(EASYTR(CLSNAME ".Button.KeepAll"));
    ui.keepAllBtn->setToolTip(EASYTR(CLSNAME ".Button.KeepAll.ToolTip"));
    ui.decideAllBtn->setText(EASYTR(CLSNAME ".Button.DecideAll"));
}

void ProgressWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        updateText();
    QWidget::changeEvent(event);
}

void ProgressWidget::onPauseResumeBtnPressed()
{
    paused_ ? resume() : pause();
}

void ProgressWidget::onCancelBtnPressed()
{
    cancel();
}

void ProgressWidget::onDetailsBtnPressed()
{
    if (!errorLogDlg_.isVisible())
        errorLogDlg_.show();
    errorLogDlg_.activateWindow();
    errorLogDlg_.raise();
}

void ProgressWidget::onReplaceAllBtnPressed()
{
    pageToMainWidget();
    emit allConflictsDecided(CES_REPLACE);
}

void ProgressWidget::onSkipAllBtnPressed()
{
    pageToMainWidget();
    emit allConflictsDecided(CES_SKIP);
}

void ProgressWidget::onKeepAllBtnPressed()
{
    pageToMainWidget();
    emit allConflictsDecided(CES_KEEP);
}

void ProgressWidget::onDecideAllBtnPressed()
{
    pageToMainWidget();
    ConflictDecisionDialog dlg(conflicts_, this);
    int ret = dlg.exec();
    if (ret == QDialog::Accepted)
    {
        // 如果所有冲突项都使用None/Skip策略则直接发送“对所有冲突项采用Skip策略”信号，以进行优化。
        if (optimizeEcs(conflicts_))
            emit allConflictsDecided(CES_SKIP);
        else
            emit conflictsDecided(conflicts_);
    }
    else
    {
        // 默认对所有冲突项采用Skip策略。
        emit allConflictsDecided(CES_SKIP);
    }
}

bool ProgressWidget::optimizeEcs(LinkTasks& tasks)
{
    int counter = 0;
    for (auto& task : tasks)
    {
        if (task.ces == CES_NONE || task.ces == CES_SKIP)
        {
            task.ces = CES_SKIP;
            counter++;
        }
    }
    return counter == stats_.conflicts;
}

void ProgressWidget::updateStatsDisplay()
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

void ProgressWidget::updateProgressDisplay()
{
    ui.progressBar->setValue(stats_.progress());
}

void ProgressWidget::updateSpeedRemainingTimeDisplay()
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

void ProgressWidget::updateCurrentEntryDisplay(const EntryPair& currentEntryPair)
{
    const auto& source = currentEntryPair.source;
    ui.currentEntryValue->setText(source.fileinfo.absoluteFilePath());
    ui.currentEntryValue->setToolTip(source.fileinfo.absoluteFilePath());

    ui.fileText->hide();
    ui.directoryText->hide();
    ui.symbolText->hide();

    const auto& fileinfo = source.fileinfo;
    if (fileinfo.isSymbolicLink() || fileinfo.isShortcut() || fileinfo.isBundle() || fileinfo.isJunction())
        ui.symbolText->show();
    else if (fileinfo.isFile())
        ui.fileText->show();
    else if (fileinfo.isDir())
        ui.directoryText->show();
    else ; // pass
}

void ProgressWidget::updateRemainingEntriesDisplay()
{
    ui.remainingEntriesValue->setText(QString::number(stats_.totalEntries - stats_.processedEntries));
}

void ProgressWidget::updateFailedCountDisplay()
{
    ui.failedEntriesValue->setText(QString::number(stats_.failedEntries));
    // 如果失败项数量不为0则启用errorWgt，并且立即显示。
    if (stats_.failedEntries > 0 && !ui.errorWgt->isEnabled())
    {
        ui.errorWgt->setEnabled(true);
        if (keepWhenErrorOccurred_)
            showAndActivate();
    }
}

void ProgressWidget::pageToMainWidget()
{
    ui.stackedWidget->setCurrentIndex(0);
}

void ProgressWidget::pageToEcsWidget()
{
    ui.stackedWidget->setCurrentIndex(1);
    updateEcsWidgetTipText();   // 显式更新Tip文本以同步当前冲突项数量。
    ui.replaceAllBtn->setFocus();
    ui.replaceAllBtn->setShortcut(QKeySequence::fromString("R"));
    ui.skipAllBtn->setShortcut(QKeySequence::fromString("S"));
    ui.keepAllBtn->setShortcut(QKeySequence::fromString("K"));
    ui.decideAllBtn->setShortcut(QKeySequence::fromString("D"));
    qApp->alert(this);          // 通过任务栏提醒用户需要决定冲突处理策略。
}

QString ProgressWidget::linkTypeString() const
{
    switch (linkType_)
    {
        case LT_HARDLINK:    return EASYTR("LinkType.Hardlink");
        case LT_SYMLINK:     return EASYTR("LinkType.Symlink");
        default: return "";
    }
}

void ProgressWidget::updateHeaderText1()
{
    ui.headerText1->setText(
        QString("%1 %2 %3").arg(
            linkTypeString(),
            QString::number(stats_.totalEntries),
            EASYTR(CLSNAME ".Label.HeaderText1")
        )
    );
}

void ProgressWidget::updatePauseResumeBtnIcon()
{
    if (paused_)
        ui.pauseResumeBtn->setIcon(QIcon(":/icons/play.ico"));
    else
        ui.pauseResumeBtn->setIcon(QIcon(":/icons/pause.ico"));
}

void ProgressWidget::updateCurrentEntryTypeText()
{
    ui.fileText->setText(EASYTR(CLSNAME ".EntryType.File"));
    ui.directoryText->setText(EASYTR(CLSNAME ".EntryType.Dir"));
    ui.symbolText->setText(EASYTR(CLSNAME ".EntryType.Symbol"));
}

void ProgressWidget::updateEcsWidgetTipText()
{
    ui.cesTipText->setText(QString(EASYTR(CLSNAME ".Label.EcsTipText")).arg(stats_.conflicts));
}
