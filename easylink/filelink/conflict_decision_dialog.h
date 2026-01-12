#pragma once

#include <qdialog.h>

#include "ui_conflict_decision_dialog.h"
#include "types.h"

class ConflictDecisionDialog : public QDialog
{
public:
    explicit ConflictDecisionDialog(LinkTasks& conflicts, QWidget* parent = nullptr);

protected:
    virtual void updateText();

    void onSkipSameDateSizeCbToggled();

private:
    Ui::ConflictDecisionDialog ui;
};
