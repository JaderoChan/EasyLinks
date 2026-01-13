#include "conflict_decision_dialog.h"

#include <qsortfilterproxymodel.h>

#include <easy_translate.hpp>

#include "conflict_decision_table_model.h"

#define CLSNAME "ConflictDecisionDialog"

ConflictDecisionDialog::ConflictDecisionDialog(LinkTasks& conflicts, QWidget* parent)
    : QDialog(parent), model_(new ConflictDecisionTableModel(conflicts, this))
{
    ui.setupUi(this);

    ui.tableView->setModel(model_);
    ui.tableView->verticalHeader()->setMinimumSectionSize(36);
    ui.tableView->verticalHeader()->setMaximumSectionSize(36);
    ui.tableView->verticalHeader()->setSectionsClickable(false);
    ui.tableView->horizontalHeader()->setSectionsClickable(false);
    ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int i = 0; i < model_->rowCount(); ++i)
        sameDateSizeEntries_ += model_->index(i, 0).data(SAME_DATE_SIZE_ROLE).toBool() ? 1 : 0;

    connect(ui.okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui.cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui.skipSameDateSizeCb, &QCheckBox::toggled, this, &ConflictDecisionDialog::onSkipSameDateSizeCbToggled);

    updateText();
}

void ConflictDecisionDialog::updateText()
{
    setWindowTitle(EASYTR(CLSNAME ".WindowTitle"));
    ui.headerText1->setText(EASYTR(CLSNAME ".Label.HeaderText1"));
    ui.headerText2->setText(EASYTR(CLSNAME ".Label.HeaderText2"));
    ui.skipSameDateSizeCb->setText(QString(EASYTR(CLSNAME ".CheckBox.SkipSameDateSize")).arg(sameDateSizeEntries_));
    ui.okBtn->setText(EASYTR("CommonButton.Ok"));
    ui.cancelBtn->setText(EASYTR("CommonButton.Cancel"));
}

void ConflictDecisionDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        updateText();
    QDialog::changeEvent(event);
}

void ConflictDecisionDialog::onSkipSameDateSizeCbToggled()
{
    // todo
}
