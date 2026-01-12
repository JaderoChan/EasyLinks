#pragma once

#include <qabstractitemmodel.h>
#include <qfileinfo.h>
#include <qicon.h>

#include "types.h"

class ConflictDecisionTableviewModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    // Non-thread-safe
    static QIcon getFileIcon(const QFileInfo& fileinfo);

    explicit ConflictDecisionTableviewModel(LinkTasks& conflicts, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole) override;
    bool setHeaderData(
        int section,
        Qt::Orientation orientation,
        const QVariant& value,
        int role = Qt::DisplayRole) override;

private:
    static EntryConflictStrategy getECSByCheckState(Qt::CheckState source, Qt::CheckState target);

    LinkTasks& conflicts_;
};
