#pragma once

#include <qabstractitemmodel.h>
#include <qfileinfo.h>
#include <qicon.h>

#include "types.h"

#define URL_ROLE (Qt::UserRole + 1)
#define SAME_DATE_SIZE_ROLE (Qt::UserRole + 2)

class ConflictDecisionTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ConflictDecisionTableModel(LinkTasks& conflicts, QObject* parent = nullptr);

    // Non-thread-safe
    static QIcon getFileIcon(const QFileInfo& fileinfo);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override;

    void setAllSourceChecked(bool checked);
    void setAllTargetChecked(bool checked);

signals:
    // 当单个项的CheckState发生改变时发出。
    void dataCheckStateToggled(const QModelIndex& idx, bool checked);

private:
    static EntryConflictStrategy getECSByCheckState(Qt::CheckState source, Qt::CheckState target);

    LinkTasks& conflicts_;
};
