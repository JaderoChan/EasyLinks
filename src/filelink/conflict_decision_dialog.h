#pragma once

#include <qdialog.h>
#include <qevent.h>
#include <qsortfilterproxymodel.h>

#include "ui_conflict_decision_dialog.h"
#include "conflict_decision_table_model.h"
#include "types.h"

class ConflictDecisionDialog : public QDialog
{
public:
    explicit ConflictDecisionDialog(LinkTasks& conflicts, QWidget* parent = nullptr);

protected:
    virtual void updateText();

    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

    void onModelDataCheckStateToggled(const QModelIndex& idx, bool checked);
    void onSkipSameDateSizeCbToggled();

private:
    Ui::ConflictDecisionDialog ui;
    ConflictDecisionTableModel* model_ = nullptr;
    QSortFilterProxyModel* proxyModel_ = nullptr;
    LinkTasks& conflicts_;
    int checkedSources_ = 0;
    int checkedTargets_ = 0;
    int sameDateSizeEntries_ = 0;
    bool isFiltered_ = false;
    bool sourceHeaderWgtPressed_ = false;
    bool targetHeaderWgtPressed_ = false;
};
