#include "conflict_decision_dialog.h"

#include <easy_translate.hpp>

#include "conflict_decision_table_model.h"

#define CLSNAME "ConflictDecisionDialog"

ConflictDecisionDialog::ConflictDecisionDialog(LinkTasks& conflicts, QWidget* parent)
    : QDialog(parent),
    model_(new ConflictDecisionTableModel(conflicts, this)),
    proxyModel_(new QSortFilterProxyModel(this))
{
    ui.setupUi(this);

    proxyModel_->setSourceModel(model_);
    proxyModel_->setFilterRole(SAME_DATE_SIZE_ROLE);
    proxyModel_->setFilterKeyColumn(0);
    ui.tableView->setModel(proxyModel_);
    ui.tableView->verticalHeader()->setSectionsClickable(false);
    ui.tableView->horizontalHeader()->setSectionsClickable(false);
    ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    sameDateSizeEntries_ = model_->match(model_->index(0, 0), SAME_DATE_SIZE_ROLE, true, -1).size();

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
    if (!isFiltered)
        proxyModel_->setFilterFixedString("false");
    else
        proxyModel_->setFilterFixedString("");
    isFiltered = !isFiltered;
}
