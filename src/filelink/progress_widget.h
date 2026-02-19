#pragma once

#include <qevent.h>
#include <qtimer.h>
#include <qwidget.h>

#include "ui_progress_widget.h"
#include "error_log_dialog.h"
#include "types.h"

class ProgressWidget : public QWidget
{
    Q_OBJECT

public:
    ProgressWidget(
        LinkType linkType,
        const QString& sourceDir,   // For header area info display.
        const QString& targetDir,   // For header area info display.
        bool keepWhenErrorOccurred,
        QWidget* parent = nullptr);
    ~ProgressWidget();

    void pause();
    void resume();
    void cancel();

    void updateProgress(const EntryPair& currentEntryPair, const LinkStats& stats);
    void appendErrorLog(LinkType linkType, const EntryPair& entryPair, const QString& errorMsg);
    void decideConflicts(const LinkTasks& conflicts);

    void onWorkFinished();

    void showAndActivate();
    void laterShowAndActivate(int ms);

signals:
    void pauseTriggered();
    void resumeTriggered();
    void cancelTriggered();
    void allConflictsDecided(ConflictingEntryStrategy ConflictPolicy);
    void conflictsDecided(LinkTasks tasks);

protected:
    virtual void updateText();
    void changeEvent(QEvent* event) override;

    void onPauseResumeBtnPressed();
    void onCancelBtnPressed();
    void onDetailsBtnPressed();
    void onReplaceAllBtnPressed();
    void onSkipAllBtnPressed();
    void onKeepAllBtnPressed();
    void onDecideAllBtnPressed();

private:
    // 更改所有None策略的冲突项为Skip策略。
    // 如果所有冲突项均为None或Skip策略则返回true，否则返回false。
    bool optimizeEcs(LinkTasks& tasks);

    void updateStatsDisplay();

    void updateProgressDisplay();
    void updateSpeedRemainingTimeDisplay();
    void updateCurrentEntryDisplay(const EntryPair& currentEntryPair);
    void updateRemainingEntriesDisplay();
    void updateFailedCountDisplay();

    void pageToMainWidget();
    void pageToEcsWidget();

    QString linkTypeString() const;
    void updateHeaderText1();
    void updatePauseResumeBtnIcon();
    void updateCurrentEntryTypeText();
    void updateEcsWidgetTipText();

private:
    Ui::ProgressWidget ui;
    ErrorLogDialog errorLogDlg_;

    LinkType linkType_;
    LinkStats stats_;
    LinkTasks conflicts_;

    QTimer speedRemainingTimeUpdateTimer_;

    int lastProcessedEntries_ = 0;
    int lastTotalEntries_ = 0;
    int speed_ = 0;

    bool paused_ = false;
    bool keepWhenErrorOccurred_ = false;
};
