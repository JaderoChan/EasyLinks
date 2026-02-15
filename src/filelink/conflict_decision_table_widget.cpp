#include "conflict_decision_table_widget.h"

#include <qdir.h>
#include <qfile.h>
#include <qheaderview.h>
#include <qlayout.h>
#include <qscrollbar.h>

#include <easy_translate.hpp>

#define CLSNAME "ConflictDecisionTableWidget"
#define HEADER_TEXT_FORMAT_STRING \
"<html><head/><body><p>%1 <a href='file:///%2'><span style='color:rgba(0, 100, 180, 216);" \
"text-decoration:none;'>%3</span></a></p></body></html>"

ConflictDecisionTableWidget::ConflictDecisionTableWidget(LinkTasks& conflicts, QWidget* parent)
    : QWidget(parent), conflicts_(conflicts),
    sourceHeader_(this), targetHeader_(this),
    view_(this), model_(conflicts, this), proxyModel_(this)
{
    setupUi();

    connect(&sourceHeader_, &ConflictDecisionTableHeader::clicked, this, &ConflictDecisionTableWidget::onSourceHeaderClicked);
    connect(&targetHeader_, &ConflictDecisionTableHeader::clicked, this, &ConflictDecisionTableWidget::onTargetHeaderClicked);
    connect(&proxyModel_, &QSortFilterProxyModel::dataChanged, this, &ConflictDecisionTableWidget::onModelDataChanged);
    connect(&proxyModel_, &QSortFilterProxyModel::modelReset, this, &ConflictDecisionTableWidget::onModelDataChanged);

    updateText();
}

int ConflictDecisionTableWidget::sameDateSizeEntries() const
{
    return model_.match(model_.index(0, 0), SAME_DATE_SIZE_ROLE, true, -1).size();
}

void ConflictDecisionTableWidget::setSkipSameDateSize(bool skip)
{
    // 首先重置所有项的CheckState。
    {
        int rows = proxyModel_.rowCount();
        model_.beginBatchSet();
        for (int row = 0; row < rows; ++row)
        {
            auto sourceRow = proxyModel_.mapToSource(proxyModel_.index(row, 0)).row();
            model_.setEcs(sourceRow, CES_NONE);
        }
        model_.endBatchSet();
    }

    proxyModel_.setFilterFixedString(skip ? "false" : "");
}

void ConflictDecisionTableWidget::setAllSourcesChecked(bool checked)
{
    int rows = proxyModel_.rowCount();
    model_.beginBatchSet();
    for (int row = 0; row < rows; ++row)
    {
        auto sourceIdx = proxyModel_.mapToSource(proxyModel_.index(row, 0));
        model_.setChecked(sourceIdx, checked);
    }
    model_.endBatchSet();
}

void ConflictDecisionTableWidget::setAllTargetsChecked(bool checked)
{
    int rows = proxyModel_.rowCount();
    model_.beginBatchSet();
    for (int row = 0; row < rows; ++row)
    {
        auto sourceIdx = proxyModel_.mapToSource(proxyModel_.index(row, 1));
        model_.setChecked(sourceIdx, checked);
    }
    model_.endBatchSet();
}

void ConflictDecisionTableWidget::updateText()
{
    if (!conflicts_.isEmpty())
    {
        const auto& source = conflicts_.front().entryPair.source.fileinfo;
        sourceHeader_.setText(QString(HEADER_TEXT_FORMAT_STRING).arg(
            EASYTR(CLSNAME ".Label.SourceHeaderText"),
            source.absolutePath(),
            source.absoluteDir().isRoot() ? source.absoluteDir().path() : source.absoluteDir().dirName()));
        sourceHeader_.setToolTip(source.absolutePath());

        const auto& target = conflicts_.front().entryPair.target.fileinfo;
        targetHeader_.setText(QString(HEADER_TEXT_FORMAT_STRING).arg(
            EASYTR(CLSNAME ".Label.TargetHeaderText"),
            target.absolutePath(),
            target.absoluteDir().isRoot() ? target.absoluteDir().path() : target.absoluteDir().dirName()));
        targetHeader_.setToolTip(target.absolutePath());
    }
}

void ConflictDecisionTableWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        updateText();
    QWidget::changeEvent(event);
}

void ConflictDecisionTableWidget::onModelDataChanged()
{
    int checkedSources = proxyModel_.match(proxyModel_.index(0, 0), Qt::CheckStateRole, Qt::Checked, -1).size();
    if (checkedSources == 0)
        sourceHeader_.setCheckState(Qt::Unchecked);
    else if (checkedSources == proxyModel_.rowCount())
        sourceHeader_.setCheckState(Qt::Checked);
    else
        sourceHeader_.setCheckState(Qt::PartiallyChecked);

    int checkedTargets = proxyModel_.match(proxyModel_.index(0, 1), Qt::CheckStateRole, Qt::Checked, -1).size();
    if (checkedTargets == 0)
        targetHeader_.setCheckState(Qt::Unchecked);
    else if (checkedTargets == proxyModel_.rowCount())
        targetHeader_.setCheckState(Qt::Checked);
    else
        targetHeader_.setCheckState(Qt::PartiallyChecked);
}

void ConflictDecisionTableWidget::onSourceHeaderClicked()
{
    switch (sourceHeader_.checkState())
    {
        case Qt::Unchecked: // Fallthrough
        case Qt::PartiallyChecked:
            setAllSourcesChecked(true);
            break;
        case Qt::Checked:
            setAllSourcesChecked(false);
            break;
        default:
            break;
    }
}

void ConflictDecisionTableWidget::onTargetHeaderClicked()
{
    switch (targetHeader_.checkState())
    {
        case Qt::Unchecked: // Fallthrough
        case Qt::PartiallyChecked:
            setAllTargetsChecked(true);
            break;
        case Qt::Checked:
            setAllTargetsChecked(false);
            break;
        default:
            break;
    }
}

void ConflictDecisionTableWidget::setupUi()
{
    proxyModel_.setSourceModel(&model_);
    proxyModel_.setFilterRole(SAME_DATE_SIZE_ROLE);
    proxyModel_.setFilterKeyColumn(0);

    view_.setModel(&proxyModel_);
    view_.setShowGrid(false);
    view_.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    view_.setFrameShape(QFrame::NoFrame);
    view_.setIconSize(QSize(36, 36));
    view_.horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view_.horizontalHeader()->hide();
    view_.verticalHeader()->hide();
    view_.verticalHeader()->setDefaultSectionSize(48);
    view_.verticalHeader()->setMinimumSectionSize(48);
    view_.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    view_.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    {
        QFile file(":/stylesheets/table_view.qss");
        if (file.open(QFile::ReadOnly))
        {
            QString styleSheet = file.readAll();
            view_.setStyleSheet(styleSheet);
            file.close();
        }
    }

    // todo
    // 待优化：对齐表格单元格的左内边距，暂时没有找到其他解决办法，所以目前只能硬编码。
    sourceHeader_.setContentsMargins(3, 0, 0, 0);
    targetHeader_.setContentsMargins(3, 0, 0, 0);

    auto headerWgt = new QWidget(this);
    headerWgt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto headerLayout = new QHBoxLayout(headerWgt);
    headerLayout->setContentsMargins(0, 0, view_.verticalScrollBar()->width(), 0);
    headerLayout->setSpacing(0);
    headerLayout->addWidget(&sourceHeader_);
    headerLayout->addWidget(&targetHeader_);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->addWidget(headerWgt);
    layout->addWidget(&view_);

    layout->setContentsMargins(24, 12, 24 - view_.verticalScrollBar()->width(), 0);
}
