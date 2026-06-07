#include "file_review_dialog.h"

#include <easy_translate.hpp>
#include <qsignalblocker.h>
#include <qtreewidget.h>

FileReviewDialog::FileReviewDialog(QList<QFileInfoList>& entryGroups, QWidget* parent)
    : QDialog(parent), entryGroups_(entryGroups)
{
    ui.setupUi(this);

    ui.treeWidget->setColumnCount(1);
    ui.treeWidget->setHeaderHidden(true);
    ui.selectAllCheckBox->setTristate(true);

    connect(ui.confirmButton, &QPushButton::clicked, this, &QDialog::accept);

    connect(ui.treeWidget, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* item, int column)
    {
        if (column != 0 || isSyncingCheckState_)
            return;

        isSyncingCheckState_ = true;
        {
            QSignalBlocker treeBlocker(ui.treeWidget);

            if (item->parent() == nullptr)
            {
                // 父级更改子级状态
                if (item->checkState(0) == Qt::Checked || item->checkState(0) == Qt::Unchecked)
                    setChildrenCheckState(item, item->checkState(0));
            }
            else
            {
                // 子级更改父级状态
                updateParentCheckState(item->parent());
            }
        }

        updateSelectAllCheckState();
        isSyncingCheckState_ = false;
    });

    connect(ui.selectAllCheckBox, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state)
    {
        if (isSyncingCheckState_ || state == Qt::PartiallyChecked)
            return;

        isSyncingCheckState_ = true;
        {
            QSignalBlocker treeBlocker(ui.treeWidget);

            for (int i = 0; i < ui.treeWidget->topLevelItemCount(); ++i)
            {
                auto* parentItem = ui.treeWidget->topLevelItem(i);
                parentItem->setCheckState(0, state);
                setChildrenCheckState(parentItem, state);
            }
        }

        isSyncingCheckState_ = false;
        updateSelectAllCheckState();
    });

    buildTree();

    updateText();
}

void FileReviewDialog::done(int result)
{
    if (result == QDialog::Accepted)
        applyConfirmedSelection();

    emit reviewFinished();
    QDialog::done(result);
}

void FileReviewDialog::buildTree()
{
    filecount_ = 0;
    ui.treeWidget->clear();

    isSyncingCheckState_ = true;
    {
        QSignalBlocker treeBlocker(ui.treeWidget);

        for (int groupIndex = 0; groupIndex < entryGroups_.size(); ++groupIndex)
        {
            const auto& group = entryGroups_[groupIndex];
            auto* groupItem = new QTreeWidgetItem(ui.treeWidget);

            groupItem->setText(0, QString::number(groupIndex));
            groupItem->setToolTip(0, QString::number(groupIndex));
            groupItem->setFlags(
                groupItem->flags() | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
            groupItem->setCheckState(0, Qt::Checked);

            for (int fileIndex = 0; fileIndex < group.size(); ++fileIndex)
            {
                const auto& fileInfo = group[fileIndex];
                auto* fileItem = new QTreeWidgetItem(groupItem);

                fileItem->setText(0, fileInfo.fileName());
                fileItem->setToolTip(0, fileInfo.filePath());
                fileItem->setFlags(
                    fileItem->flags() | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
                fileItem->setCheckState(0, Qt::Checked);

                filecount_++;
            }
        }
    }

    ui.treeWidget->expandAll();
    isSyncingCheckState_ = false;
    updateSelectAllCheckState();
}

void FileReviewDialog::setChildrenCheckState(QTreeWidgetItem* parentItem, Qt::CheckState state)
{
    for (int i = 0; i < parentItem->childCount(); ++i)
        parentItem->child(i)->setCheckState(0, state);
}

void FileReviewDialog::updateParentCheckState(QTreeWidgetItem* parentItem)
{
    int checkedCount = 0;
    int uncheckedCount = 0;

    for (int i = 0; i < parentItem->childCount(); ++i)
    {
        const auto state = parentItem->child(i)->checkState(0);
        if (state == Qt::Checked)        checkedCount++;
        else if (state == Qt::Unchecked) uncheckedCount++;
    }

    if (checkedCount == parentItem->childCount())        parentItem->setCheckState(0, Qt::Checked);
    else if (uncheckedCount == parentItem->childCount()) parentItem->setCheckState(0, Qt::Unchecked);
    else                                                 parentItem->setCheckState(0, Qt::PartiallyChecked);
}

void FileReviewDialog::updateSelectAllCheckState()
{
    int checkedLeafCount = 0;
    int leafCount = 0;

    for (int i = 0; i < ui.treeWidget->topLevelItemCount(); ++i)
    {
        auto* groupItem = ui.treeWidget->topLevelItem(i);
        for (int j = 0; j < groupItem->childCount(); ++j)
        {
            leafCount++;
            if (groupItem->child(j)->checkState(0) == Qt::Checked)
                checkedLeafCount++;
        }
    }

    Qt::CheckState targetState = Qt::Unchecked;
    if (leafCount > 0)
    {
        if (checkedLeafCount == leafCount) targetState = Qt::Checked;
        else if (checkedLeafCount > 0)     targetState = Qt::PartiallyChecked;
    }

    const QSignalBlocker selectAllBlocker(ui.selectAllCheckBox);
    ui.selectAllCheckBox->setCheckState(targetState);
}

void FileReviewDialog::applyConfirmedSelection()
{
    const int groupCount = std::min(entryGroups_.size(), ui.treeWidget->topLevelItemCount());

    for (int groupIndex = groupCount - 1; groupIndex >= 0; --groupIndex)
    {
        auto& group = entryGroups_[groupIndex];
        auto* groupItem = ui.treeWidget->topLevelItem(groupIndex);
        const int childCount = std::min(group.size(), groupItem->childCount());

        for (int childIndex = childCount - 1; childIndex >= 0; --childIndex)
        {
            auto* fileItem = groupItem->child(childIndex);
            if (fileItem->checkState(0) == Qt::Checked)
                group.removeAt(childIndex);
        }
    }
}

void FileReviewDialog::updateText()
{
    setWindowTitle(EASYTR("Review files"));
    ui.tipTextLabel->setText(
        QString(EASYTR("%1 files and %2 groups"))
        .arg(filecount_)
        .arg(entryGroups_.count()));
    ui.selectAllCheckBox->setText(EASYTR("Select all"));
    ui.confirmButton->setText(EASYTR("Confirm"));
}
