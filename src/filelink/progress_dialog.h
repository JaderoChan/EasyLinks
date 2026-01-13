#pragma once

#include <qdialog.h>
#include <qevent.h>
#include <qtimer.h>

#include "ui_progress_dialog.h"
#include "error_log_dialog.h"
#include "types.h"

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    ProgressDialog(
        LinkType linkType,
        const QString& sourceDir,   // For header area info display.
        const QString& targetDir,   // For header area info display.
        QWidget* parent = nullptr);
    ~ProgressDialog();

    void pause();
    void resume();
    void cancel();

    void updateProgress(const EntryPair& currentEntryPair, const LinkStats& stats);
    void appendErrorLog(LinkType linkType, const EntryPair& entryPair, const QString& errorMsg);
    void decideConflicts(const LinkTasks& conflicts);

    void onWorkFinished();

    void laterShow(int ms);

signals:
    void pauseTriggered();
    void resumeTriggered();
    void cancelTriggered();
    void allConflictsDecided(EntryConflictStrategy ConflictPolicy);
    void conflictsDecided(LinkTasks tasks);

protected:
    virtual void updateText();

    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    void onPauseResumeBtnPressed();
    void onCancelBtnPressed();
    void onDetailsBtnPressed();
    void onReplaceAllBtnPressed();
    void onSkipAllBtnPressed();
    void onKeepAllBtnPressed();
    void onDecideAllBtnPressed();

private:
    // 更改tasks中所有None策略的条目为Skip策略。
    // 如果所有task均为None或Skip则返回true，否则返回false。
    bool normalizeECS(LinkTasks& tasks);

    void updateStatsDisplay();

    void updateProgressDisplay();
    void updateSpeedRemainingTimeDisplay();
    void updateCurrentEntryDisplay(const EntryPair& currentEntryPair);
    void updateRemainingEntriesDisplay();
    void updateFailedCountDisplay();

    void pageToMainWidget();
    void pageToECSWidget();

    QString linkTypeString() const;
    void updateHeaderText1();
    void updatePauseResumeBtnIcon();
    void updateCurrentEntryTypeText();
    void updateECSWidgetTipText();

private:
    Ui::ProgressDialog ui;
    ErrorLogDialog* errorLogDlg_;

    LinkType linkType_;
    LinkStats stats_;
    LinkTasks conflicts_;

    QTimer speedRemainingTimeUpdateTimer_;

    int lastProcessedEntries_ = 0;
    int lastTotalEntries_ = 0;
    int speed_ = 0;

    bool paused_ = false;
};
