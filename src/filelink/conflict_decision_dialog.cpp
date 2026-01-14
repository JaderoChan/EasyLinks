#include "conflict_decision_dialog.h"

#include <qdir.h>
#include <qscrollbar.h>

#include <easy_translate.hpp>

#define CLSNAME "ConflictDecisionDialog"
#define HEADER_TEXT_FORMAT_STRING \
    "<html><head/><body><p>%1 <a href='%2'><span style='color:rgba(0, 100, 180, 216);" \
    "text-decoration:none;'>%3</span></a></p></body></html>"

ConflictDecisionDialog::ConflictDecisionDialog(LinkTasks& conflicts, QWidget* parent)
    : QDialog(parent),
    model_(new ConflictDecisionTableModel(conflicts, this)),
    proxyModel_(new QSortFilterProxyModel(this)),
    conflicts_(conflicts)
{
    ui.setupUi(this);

    if (!conflicts.isEmpty())
    {
        const auto& source = conflicts.front().entryPair.source.fileinfo;
        ui.sourceHeaderText->setText(QString(HEADER_TEXT_FORMAT_STRING).arg(
            EASYTR(CLSNAME ".Label.SourceHeaderText"),
            source.absolutePath(),
            source.absoluteDir().isRoot() ? source.absoluteDir().path() : source.absoluteDir().dirName()));
        const auto& target = conflicts.front().entryPair.target.fileinfo;
        ui.targetHeaderText->setText(QString(HEADER_TEXT_FORMAT_STRING).arg(
            EASYTR(CLSNAME ".Label.TargetHeaderText"),
            target.absolutePath(),
            target.absoluteDir().isRoot() ? target.absoluteDir().path() : target.absoluteDir().dirName()));
    }

    ui.sourceHeaderWgt->installEventFilter(this);
    ui.targetHeaderWgt->installEventFilter(this);

    ui.sourceHeaderCb->setFocusPolicy(Qt::NoFocus);
    ui.targetHeaderCb->setFocusPolicy(Qt::NoFocus);
    ui.sourceHeaderCb->installEventFilter(this);
    ui.targetHeaderCb->installEventFilter(this);
    ui.sourceHeaderCb->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui.targetHeaderCb->setAttribute(Qt::WA_TransparentForMouseEvents);

    proxyModel_->setSourceModel(model_);
    proxyModel_->setFilterRole(SAME_DATE_SIZE_ROLE);
    proxyModel_->setFilterKeyColumn(0);
    ui.tableView->setModel(proxyModel_);
    ui.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui.tableView->verticalScrollBar()->setFixedWidth(ui.tableWgt->layout()->contentsMargins().left());

    sameDateSizeEntries_ = model_->match(model_->index(0, 0), SAME_DATE_SIZE_ROLE, true, -1).size();

    connect(model_, &ConflictDecisionTableModel::dataCheckStateToggled, this, &ConflictDecisionDialog::onModelDataCheckStateToggled);

    connect(ui.okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui.cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui.skipSameDateSizeCb, &QCheckBox::toggled, this, &ConflictDecisionDialog::onSkipSameDateSizeCbToggled);

    updateText();
}

void ConflictDecisionDialog::updateText()
{
    setWindowTitle(QString(EASYTR(CLSNAME ".WindowTitle")).arg(conflicts_.size()));
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

bool ConflictDecisionDialog::eventFilter(QObject* obj, QEvent* event)
{
    auto wgt = qobject_cast<QWidget*>(obj);
    if (wgt)
    {
        // 禁用“模拟表头”可选框的交互。
        if (wgt == ui.sourceHeaderCb || wgt == ui.targetHeaderCb)
        {
            if (event->type() == QEvent::MouseButtonPress ||
                event->type() == QEvent::MouseButtonRelease ||
                event->type() == QEvent::MouseButtonDblClick)
            return true;
        }

        // 实现点击“模拟表头”窗体时也能够切换可选框状态。
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
        {
            QMouseEvent* e = dynamic_cast<QMouseEvent*>(event);
            if (e)
            {
                if (e->button() == Qt::LeftButton && wgt->rect().contains(e->pos()))
                {
                    if (wgt == ui.sourceHeaderWgt)
                        sourceHeaderWgtPressed_ = true;
                    else if (wgt == ui.targetHeaderWgt)
                        targetHeaderWgtPressed_ = true;
                    return true;
                }
            }
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent* e = dynamic_cast<QMouseEvent*>(event);
            if (e)
            {
                if (e->button() == Qt::LeftButton && wgt->rect().contains(e->pos()))
                {
                    if (wgt == ui.sourceHeaderWgt && sourceHeaderWgtPressed_)
                    {
                        switch (ui.sourceHeaderCb->checkState())
                        {
                            case Qt::Unchecked: // Fallthorugh
                            case Qt::PartiallyChecked:
                                ui.sourceHeaderCb->setCheckState(Qt::Checked);
                                model_->setAllSourceChecked(true);
                                checkedSources_ = conflicts_.size();
                                break;
                            case Qt::Checked:
                                ui.sourceHeaderCb->setCheckState(Qt::Unchecked);
                                model_->setAllSourceChecked(false);
                                checkedSources_ = 0;
                                break;
                            default:
                                break;
                        }
                    }
                    else if (wgt == ui.targetHeaderWgt && targetHeaderWgtPressed_)
                    {
                        switch (ui.targetHeaderCb->checkState())
                        {
                            case Qt::Unchecked: // Fallthorugh
                            case Qt::PartiallyChecked:
                                ui.targetHeaderCb->setCheckState(Qt::Checked);
                                model_->setAllTargetChecked(true);
                                checkedTargets_ = conflicts_.size();
                                break;
                            case Qt::Checked:
                                ui.targetHeaderCb->setCheckState(Qt::Unchecked);
                                model_->setAllTargetChecked(false);
                                checkedTargets_ = 0;
                                break;
                            default:
                                break;
                        }
                    }
                    sourceHeaderWgtPressed_ = false;
                    targetHeaderWgtPressed_ = false;
                    return true;
                }
            }
        }
    }
    return false;
}

// 当用户更改单个数据项的CheckState时，同步表头的CheckState。
void ConflictDecisionDialog::onModelDataCheckStateToggled(const QModelIndex& idx, bool checked)
{
    switch (idx.column())
    {
        case 0:
        {
            checkedSources_ += (checked ? 1 : -1);
            if (checkedSources_ == 0)
                ui.sourceHeaderCb->setCheckState(Qt::Unchecked);
            else if (checkedSources_ == conflicts_.size())
                ui.sourceHeaderCb->setCheckState(Qt::Checked);
            else
                ui.sourceHeaderCb->setCheckState(Qt::PartiallyChecked);
            break;
        }
        case 1:
        {
            checkedTargets_ += (checked ? 1 : -1);
            if (checkedTargets_ == 0)
                ui.targetHeaderCb->setCheckState(Qt::Unchecked);
            else if (checkedTargets_ == conflicts_.size())
                ui.targetHeaderCb->setCheckState(Qt::Checked);
            else
                ui.targetHeaderCb->setCheckState(Qt::PartiallyChecked);
            break;
        }
        default:
            break;
    }
}

void ConflictDecisionDialog::onSkipSameDateSizeCbToggled()
{
    if (!isFiltered_)
    {
        proxyModel_->setFilterFixedString("false");
        model_->setAllSameDateSizeEcs(ECS_NONE);
    }
    else
    {
        proxyModel_->setFilterFixedString("");
    }
    isFiltered_ = !isFiltered_;
}
