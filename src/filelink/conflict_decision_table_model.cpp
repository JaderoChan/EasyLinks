#include "conflict_decision_table_model.h"

#include <qdir.h>
#include <qfileiconprovider.h>

ConflictDecisionTableModel::ConflictDecisionTableModel(LinkTasks& conflicts, QObject* parent)
    : QAbstractTableModel(parent), conflicts_(conflicts)
{}

QIcon ConflictDecisionTableModel::getFileIcon(const QFileInfo& fileinfo)
{
    static QFileIconProvider fileIconProvider;
    if (fileinfo.exists())
        return fileIconProvider.icon(fileinfo);
    return QIcon();
    QIcon icon;
}

int ConflictDecisionTableModel::rowCount(const QModelIndex& parent) const
{
    return conflicts_.size();
}

int ConflictDecisionTableModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant ConflictDecisionTableModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (!index.isValid() || row >= conflicts_.size() || col >= columnCount())
        return QVariant();

    const auto& conflict = conflicts_[row];
    const auto& entry = (col == 0 ? conflict.entryPair.source : conflict.entryPair.target);
    switch (role)
    {
        case Qt::DisplayRole:
            return entry.fileinfo.fileName();
        case Qt::ToolTipRole:   // Fallthrough
            return entry.fileinfo.absoluteFilePath();
        case Qt::CheckStateRole:
        {
            switch (conflict.ecs)
            {
                case ECS_NONE:      return Qt::Unchecked;
                case ECS_REPLACE:   return col == 0 ? Qt::Checked : Qt::Unchecked;
                case ECS_SKIP:      return col == 0 ? Qt::Unchecked : Qt::Checked;
                case ECS_KEEP:      return Qt::Checked;
                default:            break;
            }
            break;
        }
        case Qt::DecorationRole:
            return getFileIcon(entry.fileinfo);
        case URL_ROLE:
            return entry.fileinfo.absolutePath();
        case SAME_DATE_SIZE_ROLE:
            return (conflict.entryPair.source.size == conflict.entryPair.target.size) &&
                (conflict.entryPair.source.lastModified == conflict.entryPair.target.lastModified);
        default:
            break;
    }

    return QVariant();
}

Qt::ItemFlags ConflictDecisionTableModel::flags(const QModelIndex& index) const
{
    int row = index.row();
    int col = index.column();
    if (!index.isValid() || row >= conflicts_.size() || col >= columnCount())
        return Qt::ItemFlags();

    Qt::ItemFlags flag = QAbstractTableModel::flags(index);
    flag |= Qt::ItemIsUserCheckable;
    return flag;
}

bool ConflictDecisionTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    int row = index.row();
    int col = index.column();
    if (!index.isValid() || row >= conflicts_.size() || col >= columnCount())
        return false;

    if (role != Qt::CheckStateRole)
        return false;

    auto left = createIndex(row, 0);
    auto source = (col == 0 ?
        value.value<Qt::CheckState>() :
        data(left, Qt::CheckStateRole).value<Qt::CheckState>());
    auto right = createIndex(row, 1);
    auto target = (col == 1 ?
        value.value<Qt::CheckState>() :
        data(right, Qt::CheckStateRole).value<Qt::CheckState>());
    conflicts_[row].ecs = getECSByCheckState(source, target);

    emit dataCheckStateToggled(index, value.toBool());
    emit dataChanged(left, right, {Qt::CheckStateRole});

    return true;
}

void ConflictDecisionTableModel::setAllSourceChecked(bool checked)
{
    for (int row = 0; row < conflicts_.size(); ++row)
    {
        auto& conflict = conflicts_[row];
        Qt::CheckState source = checked ? Qt::Checked : Qt::Unchecked;
        auto target = data(createIndex(row, 1), Qt::CheckStateRole).value<Qt::CheckState>();
        conflicts_[row].ecs = getECSByCheckState(source, target);
    }

    emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, columnCount() - 1), {Qt::CheckStateRole});
}

void ConflictDecisionTableModel::setAllTargetChecked(bool checked)
{
    for (int row = 0; row < conflicts_.size(); ++row)
    {
        auto& conflict = conflicts_[row];
        Qt::CheckState target = checked ? Qt::Checked : Qt::Unchecked;
        auto source = data(createIndex(row, 0), Qt::CheckStateRole).value<Qt::CheckState>();
        conflicts_[row].ecs = getECSByCheckState(source, target);
    }

    emit dataChanged(createIndex(0, 0), createIndex(rowCount() - 1, columnCount() - 1), {Qt::CheckStateRole});
}

EntryConflictStrategy ConflictDecisionTableModel::getECSByCheckState(Qt::CheckState source, Qt::CheckState target)
{
    if (source == Qt::Unchecked && target == Qt::Unchecked)
        return ECS_NONE;
    else if (source == Qt::Checked && target == Qt::Unchecked)
        return ECS_REPLACE;
    else if (source == Qt::Unchecked && target == Qt::Checked)
        return ECS_SKIP;
    else if (source == Qt::Checked && target == Qt::Checked)
        return ECS_KEEP;
    return ECS_NONE;
}
