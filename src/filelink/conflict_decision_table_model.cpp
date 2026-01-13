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

QVariant ConflictDecisionTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section < 0 || section >= columnCount())
        return QVariant();

    if (conflicts_.isEmpty())
        return QVariant();

    const auto& aConflict = conflicts_.front();
    const auto& entry = (section == 0 ? aConflict.entryPair.source : aConflict.entryPair.target);
    switch (role)
    {
        case Qt::DisplayRole:
            return entry.fileinfo.absoluteDir().dirName();
        case Qt::CheckStateRole:
        {
            auto checkeds = match(createIndex(0, 0), Qt::CheckStateRole, Qt::Checked, -1);
            if (checkeds.size() == conflicts_.size())
                return Qt::Checked;
            else if (checkeds.size() == 0)
                return Qt::Unchecked;
            else
                return Qt::PartiallyChecked;
        }
        case Qt::ToolTipRole:   // Fallthrough
        case URL_ROLE:
            return entry.fileinfo.absolutePath();
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

    emit dataChanged(left, right, {Qt::CheckStateRole});

    return true;
}

bool ConflictDecisionTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    if (orientation != Qt::Horizontal || section < 0 || section >= columnCount())
        return false;

    if (role != Qt::CheckStateRole)
        return false;

    for (int i = 0; i < conflicts_.size(); ++i)
    {
        auto source = (section == 0 ?
            value.value<Qt::CheckState>() :
            data(createIndex(i, 0), Qt::CheckStateRole).value<Qt::CheckState>());
        auto target = (section == 1 ?
            value.value<Qt::CheckState>() :
            data(createIndex(i, 1), Qt::CheckStateRole).value<Qt::CheckState>());
        conflicts_[i].ecs = getECSByCheckState(source, target);
    }

    if (!conflicts_.isEmpty())
        emit dataChanged(createIndex(0, 0), createIndex(conflicts_.size() - 1, 1), {Qt::CheckStateRole});

    return true;
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
