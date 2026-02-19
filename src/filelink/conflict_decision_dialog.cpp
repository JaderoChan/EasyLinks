#include "conflict_decision_dialog.h"

#include <easy_translate.hpp>

#define CLSNAME "ConflictDecisionDialog"

ConflictDecisionDialog::ConflictDecisionDialog(LinkTasks& conflicts, QWidget* parent)
    : QDialog(parent), tableWgt_(conflicts, this)
{
    ui.setupUi(this);

    ui.tableWgt->layout()->addWidget(&tableWgt_);

    conflicts_ = conflicts.size();
    sameDateSizeEntries_ = tableWgt_.sameDateSizeEntries();

    connect(ui.okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui.cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui.skipSameDateSizeCb, &QCheckBox::toggled, this, &ConflictDecisionDialog::onSkipSameDateSizeCbToggled);

    updateText();
}

void ConflictDecisionDialog::updateText()
{
    setWindowTitle(QString(EASYTR(CLSNAME ".WindowTitle")).arg(conflicts_));
    ui.headerText1->setText(EASYTR(CLSNAME ".Label.HeaderText1"));
    ui.headerText2->setText(EASYTR(CLSNAME ".Label.HeaderText2"));
    ui.skipSameDateSizeCb->setText(QString(EASYTR(CLSNAME ".CheckBox.SkipSameDateSize")).arg(sameDateSizeEntries_));
    ui.okBtn->setText(EASYTR("Common.Ok"));
    ui.cancelBtn->setText(EASYTR("Common.Cancel"));
}

void ConflictDecisionDialog::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        updateText();
    QDialog::changeEvent(event);
}

void ConflictDecisionDialog::onSkipSameDateSizeCbToggled(bool skip)
{
    tableWgt_.setSkipSameDateSize(skip);
}
