#pragma once

#include <qdialog.h>
#include <qevent.h>

#include "ui_conflict_decision_dialog.h"
#include "types.h"

class ConflictDecisionDialog : public QDialog
{
public:
    explicit ConflictDecisionDialog(LinkTasks& conflicts, QWidget* parent = nullptr);

protected:
    virtual void updateText();

    void changeEvent(QEvent* event) override;

    void onSkipSameDateSizeCbToggled();

private:
    Ui::ConflictDecisionDialog ui;
    int sameDateAndSizeEntries_ = 0;
};
