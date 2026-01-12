#include "conflict_decision_dialog.h"

#include <easy_translate.hpp>

#include "conflict_decision_tableview_model.h"

#define CLSNAME "ConflictDecisionDialog"

ConflictDecisionDialog::ConflictDecisionDialog(LinkTasks& conflicts, QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    auto* model = new ConflictDecisionTableviewModel(conflicts, this);
    ui.tableView->setModel(model);
    ui.tableView->verticalHeader()->setMinimumSectionSize(36);
    ui.tableView->verticalHeader()->setMaximumSectionSize(36);
    ui.tableView->verticalHeader()->setSectionsClickable(false);
    ui.tableView->horizontalHeader()->setSectionsClickable(false);
    ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui.okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui.cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui.skipSameDateSizeCb, &QCheckBox::toggled, this, &ConflictDecisionDialog::onSkipSameDateSizeCbToggled);

    updateText();
}

void ConflictDecisionDialog::updateText()
{
    // setWindowTitle(EASYTR(CLSNAME ".WindowTitle"));
    // ui.headerText1->setText(EASYTR(CLSNAME ".Label.HeaderText1"));
    // ui.headerText2->setText(EASYTR(CLSNAME ".Label.HeaderText2"));
    // ui.skipSameDateSizeCb->setText(EASYTR(CLSNAME ".CheckBox.SkipSameDateSize"));
    // ui.okBtn->setText(EASYTR("CommonButton.Ok"));
    // ui.cancelBtn->setText(EASYTR("CommonButton.Cancel"));
    setWindowTitle(EASYTR("Which files do you want to keep?"));
    ui.headerText1->setText(EASYTR("Which files do you want to keep?"));
    ui.headerText2->setText(EASYTR("If you select two files, a number will be added to the name of the copied file."));
    ui.skipSameDateSizeCb->setText(EASYTR("Skip 0 files with the same date and size (S̲)"));
    ui.okBtn->setText(EASYTR("Ok"));
    ui.cancelBtn->setText(EASYTR("Cancel"));
}

void ConflictDecisionDialog::onSkipSameDateSizeCbToggled()
{
    // todo
    // ui.tableView.
}
