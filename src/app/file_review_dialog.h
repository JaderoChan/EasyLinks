#pragma once

#include <qdialog.h>
#include <qfileinfo.h>
#include <qlist.h>

#include "ui_file_review_dialog.h"

class FileReviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileReviewDialog(QList<QFileInfoList>& entryGroups, QWidget* parent = nullptr);

signals:
    void reviewFinished();

protected:
    void done(int result) override;
    void updateText();

private:
    void buildTree();
    void setChildrenCheckState(QTreeWidgetItem* parentItem, Qt::CheckState state);
    void updateParentCheckState(QTreeWidgetItem* parentItem);
    void updateSelectAllCheckState();
    void applyConfirmedSelection();

    Ui::FileReviewDialog ui;
    QList<QFileInfoList>& entryGroups_;
    uint64_t filecount_ = 0;
    bool isSyncingCheckState_ = false;
};
