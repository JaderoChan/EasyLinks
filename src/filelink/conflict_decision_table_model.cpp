#include "conflict_decision_table_model.h"

#include <qdir.h>
#include <qfileiconprovider.h>

ConflictDecisionTableModel::ConflictDecisionTableModel(LinkTasks& conflicts, QObject* parent)
    : QAbstractTableModel(parent), conflicts_(conflicts)
{}

// TODO: Windows下获取位于磁盘根目录的可执行文件图标失败。
QIcon ConflictDecisionTableModel::getFileIcon(const QFileInfo& fileinfo)
{
    static QFileIconProvider fileIconProvider;
    if (fileinfo.exists())
        return fileIconProvider.icon(fileinfo);
    return QIcon();
}

ConflictingEntryStrategy ConflictDecisionTableModel::getEcsByCheckState(Qt::CheckState source, Qt::CheckState target)
{
    if (source == Qt::Unchecked && target == Qt::Unchecked)
        return CES_NONE;
    else if (source == Qt::Checked && target == Qt::Unchecked)
        return CES_REPLACE;
    else if (source == Qt::Unchecked && target == Qt::Checked)
        return CES_SKIP;
    else if (source == Qt::Checked && target == Qt::Checked)
        return CES_KEEP;
    return CES_NONE;
}

int ConflictDecisionTableModel::rowCount(const QModelIndex& parent) const
{
    return conflicts_.size();
}

int ConflictDecisionTableModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant ConflictDecisionTableModel::data(const QModelIndex& idx, int role) const
{
    int row = idx.row();
    int col = idx.column();
    if (!idx.isValid() || row >= rowCount() || col >= columnCount())
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
            switch (conflict.ces)
            {
                case CES_NONE:      return Qt::Unchecked;
                case CES_REPLACE:   return col == 0 ? Qt::Checked : Qt::Unchecked;
                case CES_SKIP:      return col == 0 ? Qt::Unchecked : Qt::Checked;
                case CES_KEEP:      return Qt::Checked;
                default:            break;
            }
            break;
        }
        case Qt::DecorationRole:
            return getFileIcon(entry.fileinfo);
        case SAME_DATE_SIZE_ROLE:
        {
            auto sourcePreInfo = conflict.entryPair.source.preInfo;
            auto targetPreInfo = conflict.entryPair.target.preInfo;
            return sourcePreInfo.size == targetPreInfo.size &&
                sourcePreInfo.lastModified == targetPreInfo.lastModified;
        }
        default:
            break;
    }

    return QVariant();
}

Qt::ItemFlags ConflictDecisionTableModel::flags(const QModelIndex& idx) const
{
    int row = idx.row();
    int col = idx.column();
    if (!idx.isValid() || row >= rowCount() || col >= columnCount())
        return Qt::ItemFlags();

    Qt::ItemFlags flag = QAbstractTableModel::flags(idx);
    flag |= Qt::ItemIsUserCheckable;
    return flag;
}

bool ConflictDecisionTableModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    int row = idx.row();
    int col = idx.column();
    if (!idx.isValid() || row >= rowCount() || col >= columnCount())
        return false;

    if (role != Qt::CheckStateRole)
        return false;

    auto left = index(row, 0);
    auto source = (col == 0 ?
        value.value<Qt::CheckState>() :
        data(left, Qt::CheckStateRole).value<Qt::CheckState>());
    auto right = index(row, 1);
    auto target = (col == 1 ?
        value.value<Qt::CheckState>() :
        data(right, Qt::CheckStateRole).value<Qt::CheckState>());
    conflicts_[row].ces = getEcsByCheckState(source, target);

    emit dataChanged(left, right, {Qt::CheckStateRole});

    return true;
}

bool ConflictDecisionTableModel::setChecked(const QModelIndex& idx, bool checked)
{
    int row = idx.row();
    int col = idx.column();
    if (!idx.isValid() || row >= rowCount() || col >= columnCount())
        return false;

    static auto boolToCheckState = [](bool checked) -> Qt::CheckState
    { return checked ? Qt::Checked : Qt::Unchecked; };

    auto source = (col == 0 ?
        boolToCheckState(checked) :
        data(index(row, 0), Qt::CheckStateRole).value<Qt::CheckState>());
    auto target = (col == 1 ?
        boolToCheckState(checked) :
        data(index(row, 1), Qt::CheckStateRole).value<Qt::CheckState>());
    conflicts_[row].ces = getEcsByCheckState(source, target);

    return true;
}

bool ConflictDecisionTableModel::setEcs(int row, ConflictingEntryStrategy ces)
{
    if (row < 0 || row >= rowCount())
        return false;
    conflicts_[row].ces = ces;
    return true;
}

void ConflictDecisionTableModel::beginBatchSet()
{
    beginResetModel();
}

void ConflictDecisionTableModel::endBatchSet()
{
    endResetModel();
}
