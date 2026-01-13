#include "conflict_decision_tableview_model.h"

#include <qdir.h>
#include <qfileiconprovider.h>

// todo：表头全选功能
// todo：表头打开外部链接功能

ConflictDecisionTableviewModel::ConflictDecisionTableviewModel(LinkTasks& conflicts, QObject* parent)
    : QAbstractTableModel(parent), conflicts_(conflicts)
{}

QIcon ConflictDecisionTableviewModel::getFileIcon(const QFileInfo& fileinfo)
{
    static QFileIconProvider fileIconProvider;
    if (fileinfo.exists())
        return fileIconProvider.icon(fileinfo);
    return QIcon();
    QIcon icon;
}

int ConflictDecisionTableviewModel::rowCount(const QModelIndex& parent) const
{
    return conflicts_.size();
}

int ConflictDecisionTableviewModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant ConflictDecisionTableviewModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    int col = index.column();
    if (!index.isValid() || row >= conflicts_.size() || col >= columnCount())
        return QVariant();

    const auto& conflict = conflicts_[row];
    const auto& fileinfo = (col == 0 ? conflict.entryPair.source : conflict.entryPair.target);
    switch (role)
    {
        case Qt::DisplayRole:
            return fileinfo.fileName();
        case Qt::ToolTipRole:
            return fileinfo.absoluteFilePath();
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
            return getFileIcon(fileinfo);
        case URL_ROLE:
            return fileinfo.absolutePath();
        default:
            break;
    }

    return QVariant();
}

QVariant ConflictDecisionTableviewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section < 0 || section >= columnCount())
        return QVariant();

    if (conflicts_.isEmpty())
        return QVariant();

    const auto& aConflict = conflicts_.front();
    const auto& fileinfo = (section == 0 ? aConflict.entryPair.source : aConflict.entryPair.target);
    switch (role)
    {
        case Qt::DisplayRole:
            return fileinfo.absoluteDir().dirName();
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
            return fileinfo.absolutePath();
        default:
            break;
    }

    return QVariant();
}

Qt::ItemFlags ConflictDecisionTableviewModel::flags(const QModelIndex& index) const
{
    int row = index.row();
    int col = index.column();
    if (!index.isValid() || row >= conflicts_.size() || col >= columnCount())
        return Qt::ItemFlags();

    Qt::ItemFlags flag = QAbstractTableModel::flags(index);
    flag |= Qt::ItemIsUserCheckable;
    return flag;
}

bool ConflictDecisionTableviewModel::setData(const QModelIndex& index, const QVariant& value, int role)
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

bool ConflictDecisionTableviewModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
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

EntryConflictStrategy ConflictDecisionTableviewModel::getECSByCheckState(Qt::CheckState source, Qt::CheckState target)
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
